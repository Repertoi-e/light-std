"""
Generate dense C tables for Unicode (full range 0..0x10FFFF) and emit
initializer lists with a BMP prefix plus a conditionally compiled tail.
At compile time, defining LSTD_UNICODE_FULL_RANGE includes the tail; otherwise
only the BMP prefix is compiled, so the array length matches UNICODE_TABLE_SIZE.

Additionally emits:
- g_unicode_ccc[UNICODE_TABLE_SIZE]            (Canonical Combining Class)
- g_unicode_decomp_offsets[UNICODE_TABLE_SIZE] (0 = none; else index into g_unicode_decomp_array)
- g_unicode_decomp_array[]                     (packed: len, cp...)
- g_unicode_comp_keys[]/g_unicode_comp_values[] (composition mapping; binary-search by key)
- g_unicode_comp_count / g_unicode_decomp_array_size

Outputs: src/lstd/unicode_tables.inc
"""

import sys
import os
import urllib.request
import re

UCD_BASE = "https://www.unicode.org/Public/UCD/latest/ucd/"

FILES = {
     "UnicodeData.txt": UCD_BASE + "UnicodeData.txt",
     "DerivedCoreProperties.txt": UCD_BASE + "DerivedCoreProperties.txt",
     "Scripts.txt": UCD_BASE + "Scripts.txt",
     "CompositionExclusions.txt": UCD_BASE + "CompositionExclusions.txt",
}


def fetch(url: str) -> str:
     with urllib.request.urlopen(url) as r:
          return r.read().decode("utf-8")


def parse_unicode_data(text: str):
     data = {}
     for line in text.splitlines():
          if not line or line.startswith("#"):
               continue
          parts = line.split(";")
          if len(parts) < 15:
               continue
          cp = int(parts[0], 16)
          gc = parts[2]
          ccc = int(parts[3]) if parts[3] else 0
          decomp_raw = parts[5].strip()
          decomp = None
          if decomp_raw:
               # canonical only (skip tags like "<compat>")
               if not decomp_raw.startswith("<"):
                    decomp = [int(x, 16) for x in decomp_raw.split()]
          upper = int(parts[12], 16) if parts[12] else None
          lower = int(parts[13], 16) if parts[13] else None
          data[cp] = {
               "gc": gc,
               "upper": upper,
               "lower": lower,
               "ccc": ccc,
               "decomp": decomp,
          }
     return data


def parse_property_ranges(text: str, wanted):
     m = {k: [] for k in wanted}
     for line in text.splitlines():
          line = line.split("#", 1)[0].strip()
          if not line:
               continue
          mobj = re.match(r"^([0-9A-F]{4,6})(?:\.\.([0-9A-F]{4,6}))?\s*;\s*(\w+)$", line)
          if not mobj:
               continue
          a = int(mobj.group(1), 16)
          b = int(mobj.group(2), 16) if mobj.group(2) else a
          prop = mobj.group(3)
          if prop in m:
               m[prop].append((a, b))
     return m


def parse_scripts(text: str):
     names = []
     byname = {}
     ranges = []  # (start, end, script_id)
     for line in text.splitlines():
          line = line.split("#", 1)[0].strip()
          if not line:
               continue
          mobj = re.match(r"^([0-9A-F]{4,6})(?:\.\.([0-9A-F]{4,6}))?\s*;\s*([A-Za-z_]+)$", line)
          if not mobj:
               continue
          a = int(mobj.group(1), 16)
          b = int(mobj.group(2), 16) if mobj.group(2) else a
          name = mobj.group(3)
          if name not in byname:
               byname[name] = len(names)
               names.append(name)
          sid = byname[name]
          ranges.append((a, b, sid))
     return names, ranges


def parse_comp_exclusions(text: str):
     exc = set()
     for line in text.splitlines():
          line = line.split("#", 1)[0].strip()
          if not line:
               continue
          if re.match(r"^[0-9A-F]{4,6}$", line):
               exc.add(int(line, 16))
     return exc


GC_MAP = [
     "Lu",
     "Ll",
     "Lt",
     "Lm",
     "Lo",
     "Mn",
     "Mc",
     "Me",
     "Nd",
     "Nl",
     "No",
     "Pc",
     "Pd",
     "Ps",
     "Pe",
     "Pi",
     "Pf",
     "Po",
     "Sm",
     "Sc",
     "Sk",
     "So",
     "Zs",
     "Zl",
     "Zp",
     "Cc",
     "Cf",
     "Cs",
     "Co",
     "Cn",
]
GC_INDEX = {name: i for i, name in enumerate(GC_MAP)}

CORE_PROPS = [
     "Alphabetic",
     "White_Space",
     "Uppercase",
     "Lowercase",
     "Cased",
     "Case_Ignorable",
     "Default_Ignorable_Code_Point",
]
CORE_INDEX = {name: i for i, name in enumerate(CORE_PROPS)}


def build_tables():
     print("Downloading Unicode data...", file=sys.stderr)
     unicode_data = parse_unicode_data(fetch(FILES["UnicodeData.txt"]))
     core_ranges = parse_property_ranges(fetch(FILES["DerivedCoreProperties.txt"]), CORE_PROPS)
     script_names, script_ranges = parse_scripts(fetch(FILES["Scripts.txt"]))
     comp_excl = parse_comp_exclusions(fetch(FILES["CompositionExclusions.txt"]))

     size = 0x110000
     to_upper = [i for i in range(size)]
     to_lower = [i for i in range(size)]
     general_category = [GC_INDEX.get("Cn", 30) for _ in range(size)]
     core_props = [0 for _ in range(size)]
     script = [0 for _ in range(size)]
     ccc = [0 for _ in range(size)]
     decomp_offsets = [0 for _ in range(size)]
     decomp_array = []
     comp_pairs = {}

     for cp, rec in unicode_data.items():
          if cp >= size:
               continue
          gc = GC_INDEX.get(rec["gc"], GC_INDEX["Cn"])
          general_category[cp] = gc
          ccc[cp] = rec["ccc"]
          if rec["upper"] is not None:
               to_upper[cp] = rec["upper"]
          if rec["lower"] is not None:
               to_lower[cp] = rec["lower"]
          decomp = rec["decomp"]
          if decomp:
               decomp_offsets[cp] = len(decomp_array) + 1
               decomp_array.append(len(decomp))
               decomp_array.extend(decomp)
               if len(decomp) == 2 and cp not in comp_excl:
                    a, b = decomp
                    key = (a << 21) | b
                    comp_pairs[key] = cp

     for name, ranges in core_ranges.items():
          bit = 1 << CORE_INDEX[name]
          for a, b in ranges:
               if a >= size:
                    continue
               if b >= size:
                    b = size - 1
               for cp in range(a, b + 1):
                    core_props[cp] |= bit

     for a, b, sid in script_ranges:
          if a >= size:
               continue
          if b >= size:
               b = size - 1
          for cp in range(a, b + 1):
               script[cp] = sid

     comp_keys = sorted(comp_pairs.keys())
     comp_vals = [comp_pairs[k] for k in comp_keys]

     return (
          script_names,
          to_upper,
          to_lower,
          general_category,
          script,
          core_props,
          ccc,
          decomp_offsets,
          decomp_array,
          comp_keys,
          comp_vals,
     )


def write_split_array(f, ctype, name, values):
     bmp = values[:0x10000]
     ext = values[0x10000:]
     f.write("const %s %s[UNICODE_TABLE_SIZE] = {\n" % (ctype, name))
     f.write(",".join(str(x) for x in bmp))
     if ext:
          f.write("\n#if defined(LSTD_UNICODE_FULL_RANGE)\n,")
          f.write(",".join(str(x) for x in ext))
          f.write("\n#endif\n")
     f.write("};\n\n")


def emit_inc(path, script_names, to_upper, to_lower, general_category, script, core_props, ccc, decomp_offsets, decomp_array, comp_keys, comp_vals):
     os.makedirs(os.path.dirname(path), exist_ok=True)
     with open(path, "w", encoding="utf-8") as f:
          f.write("// Generated by tools/gen_unicode.py. Do not edit.\n")
          f.write("// Arrays are split: BMP prefix always present; supplementary tail behind #if LSTD_UNICODE_FULL_RANGE.\n\n")
          f.write(
               "static_assert(UNICODE_TABLE_SIZE == 0x10000 || UNICODE_TABLE_SIZE == 0x110000, \"UNICODE_TABLE_SIZE must be BMP or FULL\");\n\n"
          )
          write_split_array(f, "u32", "g_unicode_to_upper", to_upper)
          write_split_array(f, "u32", "g_unicode_to_lower", to_lower)
          write_split_array(f, "u8", "g_unicode_general_category", general_category)
          write_split_array(f, "u16", "g_unicode_script", script)
          write_split_array(f, "u32", "g_unicode_core_props", core_props)
          write_split_array(f, "u8", "g_unicode_ccc", ccc)
          write_split_array(f, "u32", "g_unicode_decomp_offsets", decomp_offsets)

          f.write("const u32 g_unicode_decomp_array[%d] = {\n" % (len(decomp_array)))
          if decomp_array:
               f.write(",".join(str(x) for x in decomp_array))
          f.write("};\n\n")

          f.write("const u64 g_unicode_comp_keys[%d] = {\n" % (len(comp_keys)))
          if comp_keys:
               f.write(",".join(str(x) for x in comp_keys))
          f.write("};\n")
          f.write("const u32 g_unicode_comp_values[%d] = {\n" % (len(comp_vals)))
          if comp_vals:
               f.write(",".join(str(x) for x in comp_vals))
          f.write("};\n\n")

          f.write("const u32 g_unicode_decomp_array_size = %d;\n" % (len(decomp_array)))
          f.write("const u32 g_unicode_comp_count = %d;\n\n" % (len(comp_keys)))

          f.write("const char* const g_unicode_script_names[] = {\n")
          for name in script_names:
               f.write('  "%s",\n' % name)
          f.write("  null\n};\n")


def main():
     out = os.path.join("src", "lstd", "unicode_tables.inc")
     names, up, lo, gc, sc, props, ccc, de_off, de_arr, ck, cv = build_tables()
     emit_inc(out, names, up, lo, gc, sc, props, ccc, de_off, de_arr, ck, cv)
     print("Wrote %s" % out)


if __name__ == "__main__":
     main()
