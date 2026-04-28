# BOM Conversion Scripts

## jlc_to_digikey_bom.py

Converts an existing JLC-style BOM CSV or LCSC quotation XLS into:

- a Digi-Key upload CSV that only contains safe rows with real part-number identity
- a Digi-Key candidate BOM CSV
- an unresolved-items CSV for rows that still need lookup or manual review

This is intended to preserve the current KiCad to JLC export workflow and move the Digi-Key conversion into a post-export step.

### Usage

```powershell
c:/Users/forch/GitHub/Captain-Fantastic-Control-and-Hat-Board/docs/.venv/Scripts/python.exe scripts/jlc_to_digikey_bom.py "c:/path/to/project BOM.csv"
```

LCSC quotation workbook input:

```powershell
c:/Users/forch/GitHub/Captain-Fantastic-Control-and-Hat-Board/docs/.venv/Scripts/python.exe scripts/jlc_to_digikey_bom.py "c:/Users/forch/Downloads/export_project_20260424_030448.xls"
```

Optional lookup table:

```powershell
c:/Users/forch/GitHub/Captain-Fantastic-Control-and-Hat-Board/docs/.venv/Scripts/python.exe scripts/jlc_to_digikey_bom.py "c:/path/to/project BOM.csv" --lookup-csv scripts/commodity_lookup_template.csv
```

Optional reusable cross-reference sheet:

```powershell
c:/Users/forch/GitHub/Captain-Fantastic-Control-and-Hat-Board/docs/.venv/Scripts/python.exe scripts/jlc_to_digikey_bom.py "c:/Users/forch/Downloads/export_project_20260424_030448.xls" --cross-reference-csv "c:/Users/forch/Downloads/export_project_20260424_030448_Cross_Reference.csv"
```

Optional master reusable cross-reference sheet:

```powershell
c:/Users/forch/GitHub/Captain-Fantastic-Control-and-Hat-Board/docs/.venv/Scripts/python.exe scripts/jlc_to_digikey_bom.py "c:/Users/forch/OneDrive/JLCPCB files/Pinnall Control Board/Hat Board/Hat board BOM.csv" --master-cross-reference-csv "c:/Users/forch/OneDrive/JLCPCB files/master_cross_reference.csv"
```

### Output files

For an input file named `Project BOM.csv`, the script writes:

- `Project_BOM_DigiKey_Upload.csv`
- `Project_BOM_DigiKey_Simple_Upload.csv`
- `Project_BOM_DigiKey_Candidate_BOM.csv`
- `Project_BOM_Unresolved.csv`

The files are written next to the source BOM unless `--output-dir` is provided.

### Status values

- `Auto`: manufacturer and manufacturer part number were already present and carried through.
- `Lookup`: the row looks like a commodity passive and should be resolved through a preferred Digi-Key mapping.
- `Manual_Review`: fit-critical, generic, or incomplete rows that should not be guessed.

### Cross-reference guidance

- Use `--cross-reference-csv` to feed the reusable review sheet built by `build_cross_reference_sheet.py` directly into the converter.
- Use `--master-cross-reference-csv` to feed the combined reusable master sheet built by `build_master_cross_reference_sheet.py` into the converter.
- The converter matches cross-reference sheets by `LCSC_PN` first and `Manufacturer_PN` second.
- If both `--cross-reference-csv` and `--master-cross-reference-csv` are provided, the project sheet wins and the master sheet is only used as fallback.
- If a row in the cross-reference sheet has `DigiKey_PN`, that value is treated as the preferred Digi-Key identity.
- If `DigiKey_PN` is blank but `Onshore_Preferred_PN` is populated with supplier `Digi-Key` or left blank, the converter will also treat that as a Digi-Key-safe preferred part number.
- Candidate and unresolved outputs now include `Preferred_Onshore_PN`, `Preferred_Onshore_Supplier`, and `Cross_Reference_Match` columns so you can see exactly where the mapping came from.

### Upload guidance

- Use the `*_DigiKey_Upload.csv` file for Digi-Key import tests.
- If Digi-Key keeps importing the wrong identity from a wide CSV, use `*_DigiKey_Simple_Upload.csv` first. It contains only `Manufacturer Part Number`, `Quantity`, and `Manufacturer Name` to reduce mapping ambiguity.
- Use the `*_DigiKey_Candidate_BOM.csv` file as a review sheet, not as the first upload target.
- The upload CSV intentionally omits unresolved rows so Digi-Key does not try to match reference designators like `D1` or `J4` as if they were part numbers.
- If you have an LCSC `export_project_*.xls` quotation workbook, prefer that over the raw KiCad/JLC BOM CSV. It already contains LCSC matched manufacturer part numbers and produced a much stronger Digi-Key upload file in validation.

### Current limitations

- The script does not yet fetch Digi-Key part numbers from the web.
- Commodity passives need a lookup table or manual preferred-stock assignment to fill `DigiKey_PN`.
- Connector, module, display, and other fit-critical items are intentionally conservative and stay in `Manual_Review` unless the source BOM already has reliable data.

## build_commodity_lookup_candidates.py

Scans one BOM or a directory tree of BOMs and produces a normalized passive lookup-candidates CSV.

### Usage

```powershell
c:/Users/forch/GitHub/Captain-Fantastic-Control-and-Hat-Board/docs/.venv/Scripts/python.exe scripts/build_commodity_lookup_candidates.py "c:/Users/forch/OneDrive/JLCPCB files"
```

This writes `commodity_lookup_candidates.csv` in the input directory by default.

Use this first if you do not already have a preferred Digi-Key passive list. It gives you one compact table of recurring passive specs to validate, instead of forcing you to review every project BOM separately.

## build_cross_reference_sheet.py

Builds a reusable cross-reference sheet from an LCSC `export_project_*.xls` quotation workbook.

### Usage

```powershell
c:/Users/forch/GitHub/Captain-Fantastic-Control-and-Hat-Board/docs/.venv/Scripts/python.exe scripts/build_cross_reference_sheet.py "c:/Users/forch/Downloads/export_project_20260424_030448.xls"
```

This writes `*_Cross_Reference.csv` next to the source workbook by default.

### Purpose

- Use this when Digi-Key import coverage is too low to be practical.
- The output is a deduplicated review sheet with LCSC part number, manufacturer, manufacturer part number, package, stock information, and blank columns for Digi-Key, Mouser, or another onshore preferred source.
- Fill this sheet once and reuse it across future builds instead of repeating cross-references project by project.

## build_digikey_cross_reference_sheet.py

Builds the first Digi-Key-history-seeded master cross-reference sheet from a standard Digi-Key order CSV export.

### Usage

```powershell
c:/Users/forch/GitHub/Captain-Fantastic-Control-and-Hat-Board/docs/.venv/Scripts/python.exe scripts/build_digikey_cross_reference_sheet.py "c:/path/to/digikey_order_history.csv"
```

This writes `*_DigiKey_Cross_Reference.csv` next to the source CSV by default.

### Expected input shape

The script targets the standard Digi-Key order CSV column shape:

- `Index`
- `DigiKey Part #`
- `Manufacturer Part Number`
- `Description`
- `Customer Reference`
- `Quantity`
- `Backorder`
- `Unit Price`
- `Extended Price`

### Purpose

- Use this as the Digi-Key-first seed dataset for the master sourcing sheet.
- It deduplicates rows by manufacturer part number when available, rolls up purchased quantity across repeated orders, and preserves the preferred Digi-Key ordering code.
- It intentionally leaves `LCSC_PN` and `Preferred_JLC` blank so the same sheet can be enriched later from LCSC quote workbooks.
- The standard Digi-Key CSV does not include manufacturer name or order date, so those fields are left blank or inferred conservatively until a richer export is available.

## build_master_cross_reference_sheet.py

Builds one combined reusable cross-reference sheet from a directory tree of JLCPCB project BOMs and LCSC quote workbooks.

### Usage

```powershell
c:/Users/forch/GitHub/Captain-Fantastic-Control-and-Hat-Board/docs/.venv/Scripts/python.exe scripts/build_master_cross_reference_sheet.py "c:/Users/forch/OneDrive/JLCPCB files"
```

This writes `master_cross_reference.csv` in the input directory by default.

### Supported inputs

- project BOM CSVs with columns like `Reference`, `Qty`, `Value`, `Footprint`, `LCSC`, and `Manufacturer`
- LCSC quotation `.xls` workbooks
- LCSC quotation `.xlsx` workbooks

### Purpose

- Use this when you want one master sourcing sheet instead of maintaining one cross-reference sheet per project.
- The script scans the directory tree, skips existing Digi-Key output CSVs, deduplicates repeated parts, and preserves which source files contributed each row.
- Rows from richer LCSC quote workbooks win over plain project BOM CSV rows when the same part appears in both places.