#!/usr/bin/env python3
"""
Asset Type Classification System for GXC Stock Tokens
Explicit classification to avoid legal ambiguity
"""

from enum import Enum
from dataclasses import dataclass
from typing import Optional

class AssetType(Enum):
    """Asset type classification"""
    SYNTHETIC_EQUITY = "SYNTHETIC_EQUITY"           # Price-tracking only
    CUSTODIAL_BACKED = "CUSTODIAL_BACKED"           # 1:1 backed by real shares
    ISSUER_AUTHORIZED = "ISSUER_AUTHORIZED"         # Company-issued on-chain

class SettlementType(Enum):
    """How the asset settles"""
    CASH_CRYPTO = "cash_crypto"                     # Cash or cryptocurrency
    PHYSICAL_REDEMPTION = "physical_redemption"     # Redeemable for real shares
    COMPANY_REGISTRY = "company_registry"           # On company cap table

@dataclass
class AssetClassification:
    """Complete asset classification"""
    asset_type: AssetType
    legal_ownership: bool
    voting_rights: bool
    dividend_rights: bool
    redemption_rights: bool
    settlement_type: SettlementType
    custodian: Optional[str] = None
    proof_of_reserves_url: Optional[str] = None
    
    def get_disclaimer(self) -> str:
        """Get appropriate legal disclaimer"""
        if self.asset_type == AssetType.SYNTHETIC_EQUITY:
            return """
⚠️ IMPORTANT LEGAL NOTICE

This token is a SYNTHETIC INSTRUMENT that tracks the price of a real stock.

This token:
• DOES NOT represent legal ownership of real shares
• DOES NOT provide voting rights
• DOES NOT provide dividend rights (unless explicitly paid by market maker)
• IS NOT redeemable for real shares
• IS a derivative for price exposure only

Settlement is in cash or cryptocurrency, not physical shares.

By trading this token, you acknowledge you understand it is a price-tracking 
instrument, not securities ownership.
"""
        elif self.asset_type == AssetType.CUSTODIAL_BACKED:
            return f"""
✅ CUSTODIAL-BACKED TOKEN

This token is backed 1:1 by real shares held in custody.

This token:
• REPRESENTS legal ownership of real shares
• MAY provide voting rights (check custodian terms)
• MAY provide dividend rights (check custodian terms)
• IS redeemable for real shares (subject to custodian terms)

Custodian: {self.custodian}
Proof of Reserves: {self.proof_of_reserves_url}

Settlement is physical (redemption for real shares).
"""
        elif self.asset_type == AssetType.ISSUER_AUTHORIZED:
            return """
✅ ISSUER-AUTHORIZED TOKEN

This token is issued directly by the company and represents real shares.

This token:
• REPRESENTS legal ownership of company shares
• PROVIDES voting rights
• PROVIDES dividend rights
• IS part of the company cap table

This is a legally recognized security.
"""
        return "Unknown asset type"

# Predefined classifications
SYNTHETIC_EQUITY_CLASSIFICATION = AssetClassification(
    asset_type=AssetType.SYNTHETIC_EQUITY,
    legal_ownership=False,
    voting_rights=False,
    dividend_rights=False,
    redemption_rights=False,
    settlement_type=SettlementType.CASH_CRYPTO
)

def create_custodial_backed_classification(
    custodian: str,
    proof_of_reserves_url: str,
    voting_rights: bool = False,
    dividend_rights: bool = False
) -> AssetClassification:
    """Create custodial-backed classification"""
    return AssetClassification(
        asset_type=AssetType.CUSTODIAL_BACKED,
        legal_ownership=True,
        voting_rights=voting_rights,
        dividend_rights=dividend_rights,
        redemption_rights=True,
        settlement_type=SettlementType.PHYSICAL_REDEMPTION,
        custodian=custodian,
        proof_of_reserves_url=proof_of_reserves_url
    )

def create_issuer_authorized_classification() -> AssetClassification:
    """Create issuer-authorized classification"""
    return AssetClassification(
        asset_type=AssetType.ISSUER_AUTHORIZED,
        legal_ownership=True,
        voting_rights=True,
        dividend_rights=True,
        redemption_rights=False,  # Already on cap table
        settlement_type=SettlementType.COMPANY_REGISTRY
    )

# Export
__all__ = [
    'AssetType',
    'SettlementType',
    'AssetClassification',
    'SYNTHETIC_EQUITY_CLASSIFICATION',
    'create_custodial_backed_classification',
    'create_issuer_authorized_classification'
]
