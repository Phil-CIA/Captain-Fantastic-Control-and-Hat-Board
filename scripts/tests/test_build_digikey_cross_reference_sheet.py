"""Tests for build_digikey_cross_reference_sheet utility functions."""

from __future__ import annotations

import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))

from build_digikey_cross_reference_sheet import (
    extract_package,
    extract_value_or_spec,
    find_column_map,
    normalize_description,
    parse_currency,
    parse_int,
)


class TestParseInt:
    def test_valid_int(self) -> None:
        assert parse_int("42") == 42

    def test_whitespace_stripped(self) -> None:
        assert parse_int("  10  ") == 10

    def test_invalid_returns_zero(self) -> None:
        assert parse_int("n/a") == 0

    def test_empty_returns_zero(self) -> None:
        assert parse_int("") == 0


class TestParseCurrency:
    def test_simple_price(self) -> None:
        assert parse_currency("$1.50") == "1.5"

    def test_trailing_zeros_stripped(self) -> None:
        result = parse_currency("2.50000")
        assert result == "2.5"

    def test_empty_returns_empty(self) -> None:
        assert parse_currency("") == ""

    def test_non_numeric_returns_cleaned(self) -> None:
        result = parse_currency("USD 3.14")
        assert result == "3.14"


class TestNormalizeDescription:
    def test_collapses_whitespace(self) -> None:
        assert normalize_description("  IC  capacitor  ") == "IC capacitor"

    def test_empty(self) -> None:
        assert normalize_description("") == ""


class TestExtractPackage:
    def test_0402(self) -> None:
        assert extract_package("Resistor 10k 0402 SMD") == "0402"

    def test_soic8(self) -> None:
        assert extract_package("IC SOIC-8 driver") == "SOIC-8"

    def test_sot23(self) -> None:
        assert extract_package("Transistor SOT-23") == "SOT-23"

    def test_no_match(self) -> None:
        assert extract_package("General purpose part") == ""

    def test_uppercase(self) -> None:
        result = extract_package("0805 cap 100nF")
        assert result == "0805"


class TestExtractValueOrSpec:
    def test_pf_value(self) -> None:
        assert extract_value_or_spec("Cap 100pF 0402") == "100pF"

    def test_nf_value(self) -> None:
        assert extract_value_or_spec("Cap 10nF X7R") == "10nF"

    def test_ohm_value(self) -> None:
        assert extract_value_or_spec("Res 10kohm 0805") == "10kohm"

    def test_voltage(self) -> None:
        result = extract_value_or_spec("Cap 50V rated")
        assert result == "50V"

    def test_no_match(self) -> None:
        assert extract_value_or_spec("IC dual op-amp") == ""


class TestFindColumnMap:
    def _make_fieldnames(self) -> list[str]:
        return [
            "Index",
            "DigiKey Part #",
            "Manufacturer Part Number",
            "Description",
            "Quantity",
            "Unit Price",
            "Extended Price",
            "Customer Reference",
        ]

    def test_standard_shape_maps_correctly(self) -> None:
        col_map = find_column_map(self._make_fieldnames())
        assert col_map["digikey_pn"] == "DigiKey Part #"
        assert col_map["manufacturer_pn"] == "Manufacturer Part Number"
        assert col_map["quantity"] == "Quantity"
        assert col_map["unit_price"] == "Unit Price"

    def test_alternate_column_name(self) -> None:
        fields = [
            "Digi-Key Part Number",
            "Manufacturer Part #",
            "Description",
            "Quantity",
            "Unit Price",
            "Extended Price",
        ]
        col_map = find_column_map(fields)
        assert col_map["digikey_pn"] == "Digi-Key Part Number"
        assert col_map["manufacturer_pn"] == "Manufacturer Part #"

    def test_none_fieldnames_raises(self) -> None:
        with pytest.raises(ValueError, match="header row"):
            find_column_map(None)

    def test_missing_required_column_raises(self) -> None:
        fields = ["Index", "Description", "Quantity"]
        with pytest.raises(ValueError, match="DigiKey Part"):
            find_column_map(fields)
