"""
Generate dense C tables for Unicode (full range 0..0x10FFFF) and emit
initializer lists with a BMP prefix plus a conditionally compiled tail.
At compile time, defining ARCHE_UNICODE_FULL_RANGE includes the tail; otherwise
only the BMP prefix is compiled, so the array length matches UNICODE_TABLE_SIZE.

Additionally emits:
- g_unicode_ccc[UNICODE_TABLE_SIZE]            (Canonical Combining Class)
- g_unicode_decomp_offsets[UNICODE_TABLE_SIZE] (0 = none; else index into g_unicode_decomp_array)
- g_unicode_decomp_array[]                     (packed: len, cp...)
- g_unicode_comp_keys[]/g_unicode_comp_values[] (composition mapping; binary-search by key)
- g_unicode_comp_count / g_unicode_decomp_array_size
- g_unicode_char_width[UNICODE_TABLE_SIZE]     (display width: 0, 1, or 2)

Outputs: src/arche/unicode_tables.inc
"""

import os, sys, re, urllib.request

UCD_BASE = "https://www.unicode.org/Public/UCD/latest/ucd/"

FILES = {
     "UnicodeData.txt": UCD_BASE + "UnicodeData.txt",
     "DerivedCoreProperties.txt": UCD_BASE + "DerivedCoreProperties.txt",
     "PropList.txt": UCD_BASE + "PropList.txt",
     "Scripts.txt": UCD_BASE + "Scripts.txt",
     "CompositionExclusions.txt": UCD_BASE + "CompositionExclusions.txt",
     "EastAsianWidth.txt": UCD_BASE + "EastAsianWidth.txt",
     "emoji-data.txt": "https://www.unicode.org/Public/UCD/latest/ucd/emoji/emoji-data.txt",
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

def parse_east_asian_width(text:str):
     """Parse EastAsianWidth.txt, returns dict mapping cp -> width category (F, H, W, Na, A, N)"""
     widths = {}
     for line in text.splitlines():
          line = line.split('#',1)[0].strip()
          if not line: continue
          m = re.match(r'^([0-9A-F]{4,6})(?:\.\.([0-9A-F]{4,6}))?\s*;\s*(\w+)$', line)
          if not m: continue
          a = int(m.group(1),16)
          b = int(m.group(2),16) if m.group(2) else a
          cat = m.group(3)
          for cp in range(a, b+1):
               widths[cp] = cat
     return widths

def parse_emoji_presentation(text:str):
     """Parse emoji-data.txt for Emoji_Presentation property"""
     emoji_pres = set()
     for line in text.splitlines():
          line = line.split('#',1)[0].strip()
          if not line: continue
          m = re.match(r'^([0-9A-F]{4,6})(?:\.\.([0-9A-F]{4,6}))?\s*;\s*Emoji_Presentation$', line)
          if not m: continue
          a = int(m.group(1),16)
          b = int(m.group(2),16) if m.group(2) else a
          for cp in range(a, b+1):
               emoji_pres.add(cp)
     return emoji_pres

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
     for k,v in pl.items():
          dcp[k].extend(v)
     script_names, script_ranges=parse_scripts(fetch(FILES['Scripts.txt']))
     comp_excl=parse_comp_exclusions(fetch(FILES['CompositionExclusions.txt']))
     ea_widths=parse_east_asian_width(fetch(FILES['EastAsianWidth.txt']))
     emoji_pres=parse_emoji_presentation(fetch(FILES['emoji-data.txt']))
     
     SIZE=0x110000
     to_upper=list(range(SIZE))
     to_lower=list(range(SIZE))
     general=[GC_INDEX['Cn']]*SIZE
     script=[0]*SIZE
     ccc=[0]*SIZE
     decomp_offsets=[0]*SIZE
     decomp_array=[]
     comp_pairs={}
     char_width=[1]*SIZE
     
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
     
     for a,b,sid in script_ranges:
          if a>=SIZE: continue
          if b>=SIZE: b=SIZE-1
          for cp in range(a,b+1): script[cp]=sid
     
     prop_ranges={name: coalesce(dcp.get(name,[]), SIZE) for name in CORE_PROPS}
     comp_keys=sorted(comp_pairs.keys())
     comp_vals=[comp_pairs[k] for k in comp_keys]
     
     # Build char_width table
     # Width 0: Mn, Me, Cf categories (combining marks, format chars)
     # Width 2: East Asian Fullwidth (F) and Wide (W), plus Emoji_Presentation
     # Width 1: everything else
     for cp in range(SIZE):
          gc = general[cp]
          gc_name = GC_MAP[gc] if gc < len(GC_MAP) else 'Cn'
          
          # Zero-width: combining marks (Mn, Me) and format characters (Cf)
          if gc_name in ('Mn', 'Me', 'Cf'):
               char_width[cp] = 0
          # Also zero-width for certain control characters
          elif cp < 0x20 or (cp >= 0x7F and cp < 0xA0):
               char_width[cp] = 0 if cp != 0 else 0
          else:
               ea = ea_widths.get(cp, 'N')
               # F = Fullwidth, W = Wide -> width 2
               if ea in ('F', 'W'):
                    char_width[cp] = 2
               # Emoji with default emoji presentation -> width 2
               elif cp in emoji_pres:
                    char_width[cp] = 2
               else:
                    char_width[cp] = 1
     
     return (script_names,to_upper,to_lower,general,script,prop_ranges,ccc,decomp_offsets,decomp_array,comp_keys,comp_vals,char_width)

def write_split_array(f, ctype, name, values):
     bmp=values[:0x10000]
     ext=values[0x10000:]
     f.write(f"const {ctype} {name}[UNICODE_TABLE_SIZE] = {{\n")
     f.write(','.join(str(x) for x in bmp))
     if ext:
          f.write("\n#if defined(ARCHE_UNICODE_FULL_RANGE)\n,")
          f.write(','.join(str(x) for x in ext))
          f.write("\n#endif\n")
     f.write("};\n\n")

def emit_inc(path, script_names,to_upper,to_lower,general,script,prop_ranges,ccc,decomp_offsets,decomp_array,comp_keys,comp_vals,char_width):
     os.makedirs(os.path.dirname(path), exist_ok=True)
     with open(path,'w',encoding='utf-8') as f:
          f.write('// Generated by tools/gen_unicode.py. Do not edit.\n')
          f.write('static_assert(UNICODE_TABLE_SIZE == 0x10000 || UNICODE_TABLE_SIZE == 0x110000, "UNICODE_TABLE_SIZE must be BMP or FULL");\n\n')
          write_split_array(f,'u32','g_unicode_to_upper',to_upper)
          write_split_array(f,'u32','g_unicode_to_lower',to_lower)
          write_split_array(f,'u8','g_unicode_general_category',general)
          write_split_array(f,'u16','g_unicode_script',script)
          SIZE_FULL = len(to_upper)
          prop_mask = [0]*SIZE_FULL
          for pidx, name in enumerate(CORE_PROPS):
               for a,b in prop_ranges.get(name,[]):
                    if a >= SIZE_FULL: continue
                    if b >= SIZE_FULL: b = SIZE_FULL-1
                    for cp in range(a, b+1):
                         prop_mask[cp] |= (1 << pidx)
          bmp = prop_mask[:0x10000]
          ext = prop_mask[0x10000:]
          f.write('const u64 g_unicode_prop_mask[UNICODE_TABLE_SIZE] = {\n')
          f.write(','.join(str(x) for x in bmp))
          if ext:
               f.write('\n#if defined(ARCHE_UNICODE_FULL_RANGE)\n,')
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
          f.write('  null\n};\n\n')
          write_split_array(f,'u8','g_unicode_char_width',char_width)

def main():
     out_path=os.path.join('src','arche','unicode_tables.inc')
     data=build_tables()
     emit_inc(out_path,*data)
     print('Wrote', out_path)

if __name__=='__main__':
     main()
