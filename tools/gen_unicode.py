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
from collections import defaultdict

UCD_BASE = "https://www.unicode.org/Public/UCD/latest/ucd/"

FILES = {
     "UnicodeData.txt": UCD_BASE + "UnicodeData.txt",
     "DerivedCoreProperties.txt": UCD_BASE + "DerivedCoreProperties.txt",
     "PropList.txt": UCD_BASE + "PropList.txt",
     "Scripts.txt": UCD_BASE + "Scripts.txt",
     "CompositionExclusions.txt": UCD_BASE + "CompositionExclusions.txt",
}


def fetch(url: str) -> str:
     with urllib.request.urlopen(url) as r:
          return r.read().decode("utf-8")

"""Unicode Data Generator (range-based properties)

Generates: (written to src/lstd/unicode_tables.inc)
  - g_unicode_to_upper[UNICODE_TABLE_SIZE]
  - g_unicode_to_lower[UNICODE_TABLE_SIZE]
  - g_unicode_general_category[UNICODE_TABLE_SIZE]
  - g_unicode_script[UNICODE_TABLE_SIZE]
  - g_unicode_prop_offsets[] / g_unicode_prop_ranges[] / g_unicode_prop_names[] (range-encoded properties)
  - g_unicode_ccc[UNICODE_TABLE_SIZE]
  - g_unicode_decomp_offsets[UNICODE_TABLE_SIZE]
  - g_unicode_decomp_array[] (packed: len, cp...)
  - g_unicode_comp_keys[] / g_unicode_comp_values[] + counts

Previously we emitted a dense per-code-point 64-bit property bitset. That
became too large once we needed the union of DerivedCoreProperties.txt and
PropList.txt. We now store per-property coalesced ranges. Lookup becomes a
binary search within that property's range slice.
"""

import os, sys, re, urllib.request

UCD_BASE = "https://www.unicode.org/Public/UCD/latest/ucd/"
FILES = {
     "UnicodeData.txt": UCD_BASE + "UnicodeData.txt",
     "DerivedCoreProperties.txt": UCD_BASE + "DerivedCoreProperties.txt",
     "PropList.txt": UCD_BASE + "PropList.txt",
     "Scripts.txt": UCD_BASE + "Scripts.txt",
     "CompositionExclusions.txt": UCD_BASE + "CompositionExclusions.txt",
}

GC_MAP = [
     "Lu","Ll","Lt","Lm","Lo",
     "Mn","Mc","Me",
     "Nd","Nl","No",
     "Pc","Pd","Ps","Pe","Pi","Pf","Po",
     "Sm","Sc","Sk","So",
     "Zs","Zl","Zp",
     "Cc","Cf","Cs","Co","Cn",
]
GC_INDEX = {n:i for i,n in enumerate(GC_MAP)}

CORE_PROPS = [
     # DerivedCoreProperties.txt
     "Alphabetic","Case_Ignorable","Cased","Changes_When_Casefolded","Changes_When_Casemapped",
     "Changes_When_Lowercased","Changes_When_Titlecased","Changes_When_Uppercased","Default_Ignorable_Code_Point",
     "Grapheme_Base","Grapheme_Extend","Grapheme_Link","ID_Continue","ID_Start","Lowercase","Math","Uppercase",
     "XID_Continue","XID_Start",
     # PropList.txt
     "ASCII_Hex_Digit","Bidi_Control","Dash","Deprecated","Diacritic","Extender","Hex_Digit","Hyphen",
     "IDS_Binary_Operator","IDS_Trinary_Operator","IDS_Unary_Operator","ID_Compat_Math_Continue","ID_Compat_Math_Start",
     "Ideographic","Join_Control","Logical_Order_Exception","Modifier_Combining_Mark","Noncharacter_Code_Point",
     "Other_Alphabetic","Other_Default_Ignorable_Code_Point","Other_Grapheme_Extend","Other_ID_Continue","Other_ID_Start",
     "Other_Lowercase","Other_Math","Other_Uppercase","Pattern_Syntax","Pattern_White_Space","Prepended_Concatenation_Mark",
     "Quotation_Mark","Radical","Regional_Indicator","Sentence_Terminal","Soft_Dotted","Terminal_Punctuation",
     "Unified_Ideograph","Variation_Selector","White_Space",
]

def fetch(url:str)->str:
     with urllib.request.urlopen(url) as r:
          return r.read().decode('utf-8')

def parse_unicode_data(text:str):
     data={}
     for line in text.splitlines():
          if not line or line.startswith('#'): continue
          parts=line.split(';')
          if len(parts)<15: continue
          cp=int(parts[0],16)
          rec={
               'gc':parts[2],
               'ccc':int(parts[3]) if parts[3] else 0,
               'upper':int(parts[12],16) if parts[12] else None,
               'lower':int(parts[13],16) if parts[13] else None,
               'decomp':None
          }
          raw=parts[5].strip()
          if raw and not raw.startswith('<'):
               rec['decomp']=[int(x,16) for x in raw.split()]
          data[cp]=rec
     return data

def parse_property_ranges(text:str, names):
     out={k:[] for k in names}
     for line in text.splitlines():
          line=line.split('#',1)[0].strip()
          if not line: continue
          m=re.match(r'^([0-9A-F]{4,6})(?:\.\.([0-9A-F]{4,6}))?\s*;\s*(\w+)$', line)
          if not m: continue
          a=int(m.group(1),16)
          b=int(m.group(2),16) if m.group(2) else a
          prop=m.group(3)
          if prop in out:
               out[prop].append((a,b))
     return out

def parse_scripts(text:str):
     names=[]; byname={}; ranges=[]
     for line in text.splitlines():
          line=line.split('#',1)[0].strip()
          if not line: continue
          m=re.match(r'^([0-9A-F]{4,6})(?:\.\.([0-9A-F]{4,6}))?\s*;\s*([A-Za-z_]+)$', line)
          if not m: continue
          a=int(m.group(1),16)
          b=int(m.group(2),16) if m.group(2) else a
          name=m.group(3)
          if name not in byname:
               byname[name]=len(names)
               names.append(name)
          ranges.append((a,b,byname[name]))
     return names, ranges

def parse_comp_exclusions(text:str):
     out=set()
     for line in text.splitlines():
          line=line.split('#',1)[0].strip()
          if not line: continue
          if re.match(r'^[0-9A-F]{4,6}$', line):
               out.add(int(line,16))
     return out

def coalesce(ranges, limit):
     if not ranges: return []
     norm=[]
     for a,b in ranges:
          if a>=limit: continue
          if b>=limit: b=limit-1
          if b<a: continue
          norm.append((a,b))
     if not norm: return []
     norm.sort()
     res=[]
     ca,cb=norm[0]
     for a,b in norm[1:]:
          if a<=cb+1:
               if b>cb: cb=b
          else:
               res.append((ca,cb))
               ca,cb=a,b
     res.append((ca,cb))
     return res

def build_tables():
     print('Downloading Unicode data...', file=sys.stderr)
     unicode_data=parse_unicode_data(fetch(FILES['UnicodeData.txt']))
     dcp=parse_property_ranges(fetch(FILES['DerivedCoreProperties.txt']), CORE_PROPS)
     pl=parse_property_ranges(fetch(FILES['PropList.txt']), CORE_PROPS)
     # merge
     for k,v in pl.items():
          dcp[k].extend(v)
     script_names, script_ranges=parse_scripts(fetch(FILES['Scripts.txt']))
     comp_excl=parse_comp_exclusions(fetch(FILES['CompositionExclusions.txt']))
     SIZE=0x110000
     to_upper=list(range(SIZE))
     to_lower=list(range(SIZE))
     general=[GC_INDEX['Cn']]*SIZE
     script=[0]*SIZE
     ccc=[0]*SIZE
     decomp_offsets=[0]*SIZE
     decomp_array=[]
     comp_pairs={}
     for cp, rec in unicode_data.items():
          if cp>=SIZE: continue
          general[cp]=GC_INDEX.get(rec['gc'], GC_INDEX['Cn'])
          ccc[cp]=rec['ccc']
          if rec['upper'] is not None: to_upper[cp]=rec['upper']
          if rec['lower'] is not None: to_lower[cp]=rec['lower']
          d=rec['decomp']
          if d:
               decomp_offsets[cp]=len(decomp_array)+1
               decomp_array.append(len(d))
               decomp_array.extend(d)
               if len(d)==2 and cp not in comp_excl:
                    a,b=d
                    comp_pairs[(a<<21)|b]=cp
     # scripts
     for a,b,sid in script_ranges:
          if a>=SIZE: continue
          if b>=SIZE: b=SIZE-1
          for cp in range(a,b+1): script[cp]=sid
     # coalesce prop ranges
     prop_ranges={name: coalesce(dcp.get(name,[]), SIZE) for name in CORE_PROPS}
     comp_keys=sorted(comp_pairs.keys())
     comp_vals=[comp_pairs[k] for k in comp_keys]
     return (script_names,to_upper,to_lower,general,script,prop_ranges,ccc,decomp_offsets,decomp_array,comp_keys,comp_vals)

def write_split_array(f, ctype, name, values):
     bmp=values[:0x10000]
     ext=values[0x10000:]
     f.write(f"const {ctype} {name}[UNICODE_TABLE_SIZE] = {{\n")
     f.write(','.join(str(x) for x in bmp))
     if ext:
          f.write("\n#if defined(LSTD_UNICODE_FULL_RANGE)\n,")
          f.write(','.join(str(x) for x in ext))
          f.write("\n#endif\n")
     f.write("};\n\n")

def emit_inc(path, script_names,to_upper,to_lower,general,script,prop_ranges,ccc,decomp_offsets,decomp_array,comp_keys,comp_vals):
     os.makedirs(os.path.dirname(path), exist_ok=True)
     with open(path,'w',encoding='utf-8') as f:
          f.write('// Generated by tools/gen_unicode.py. Do not edit.\n')
          f.write('static_assert(UNICODE_TABLE_SIZE == 0x10000 || UNICODE_TABLE_SIZE == 0x110000, "UNICODE_TABLE_SIZE must be BMP or FULL");\n\n')
          write_split_array(f,'u32','g_unicode_to_upper',to_upper)
          write_split_array(f,'u32','g_unicode_to_lower',to_lower)
          write_split_array(f,'u8','g_unicode_general_category',general)
          write_split_array(f,'u16','g_unicode_script',script)
          # Dense per-code-point property bit mask (57 properties -> fits in u64)
          # Build full-range then emit split (BMP + optional tail) same as other tables
          # Bit assignment: bit i corresponds to CORE_PROPS[i]
          SIZE_FULL = len(to_upper)  # 0x110000
          prop_mask = [0]*SIZE_FULL
          for pidx, name in enumerate(CORE_PROPS):
               for a,b in prop_ranges.get(name,[]):
                    if a >= SIZE_FULL: continue
                    if b >= SIZE_FULL: b = SIZE_FULL-1
                    # set bits in [a,b]
                    # tight loop; slice assignment would be heavy; manual
                    for cp in range(a, b+1):
                         prop_mask[cp] |= (1 << pidx)
          # write split array
          bmp = prop_mask[:0x10000]
          ext = prop_mask[0x10000:]
          f.write('const u64 g_unicode_prop_mask[UNICODE_TABLE_SIZE] = {\n')
          f.write(','.join(str(x) for x in bmp))
          if ext:
               f.write('\n#if defined(LSTD_UNICODE_FULL_RANGE)\n,')
               f.write(','.join(str(x) for x in ext))
               f.write('\n#endif\n')
          f.write('};\n')
          f.write(f'const char* const g_unicode_prop_names[{len(CORE_PROPS)}] = {{\n')
          for n in CORE_PROPS:
               f.write(f'  "{n}",\n')
          f.write('};\n\n')
          write_split_array(f,'u8','g_unicode_ccc',ccc)
          write_split_array(f,'u32','g_unicode_decomp_offsets',decomp_offsets)
          f.write(f'const u32 g_unicode_decomp_array[{len(decomp_array)}] = {{\n')
          if decomp_array: f.write(','.join(str(x) for x in decomp_array))
          f.write('};\n\n')
          f.write(f'const u64 g_unicode_comp_keys[{len(comp_keys)}] = {{\n')
          if comp_keys: f.write(','.join(str(x) for x in comp_keys))
          f.write('};\n')
          f.write(f'const u32 g_unicode_comp_values[{len(comp_vals)}] = {{\n')
          if comp_vals: f.write(','.join(str(x) for x in comp_vals))
          f.write('};\n\n')
          f.write(f'const u32 g_unicode_decomp_array_size = {len(decomp_array)};\n')
          f.write(f'const u32 g_unicode_comp_count = {len(comp_keys)};\n\n')
          f.write('const char* const g_unicode_script_names[] = {\n')
          for n in script_names:
               f.write(f'  "{n}",\n')
          f.write('  null\n};\n')

def main():
     out_path=os.path.join('src','lstd','unicode_tables.inc')
     data=build_tables()
     emit_inc(out_path,*data)
     print('Wrote', out_path)

if __name__=='__main__':
     main()