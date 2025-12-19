#!/usr/bin/env python3
"""
GXC Blockchain Stock Contracts
Complete implementation supporting all 3 asset models
"""

import json
import time
import hashlib
from enum import Enum
from dataclasses import dataclass, asdict
from typing import Dict, List, Optional, Any
from decimal import Decimal

class AssetType(Enum):
    """Asset type classification"""
    SYNTHETIC_EQUITY = "SYNTHETIC_EQUITY"           # Price-tracking only (Model 1)
    CUSTODIAL_BACKED = "CUSTODIAL_BACKED"           # 1:1 backed by real shares (Model 2)
    ISSUER_AUTHORIZED = "ISSUER_AUTHORIZED"         # Company-issued on-chain (Model 3)

class SettlementType(Enum):
    """How the asset settles"""
    CASH_CRYPTO = "cash_crypto"                     # Cash or cryptocurrency
    PHYSICAL_REDEMPTION = "physical_redemption"     # Redeemable for real shares
    COMPANY_REGISTRY = "company_registry"           # On company cap table

@dataclass
class StockContract:
    """Complete stock contract with all asset types supported"""
    contract_id: str
    asset_type: AssetType
    symbol: str
    company_name: str
    exchange: str
    token_supply: int
    price_source: str
    legal_ownership: bool
    settlement_type: SettlementType
    voting_rights: bool
    dividend_rights: bool
    redemption_rights: bool
    market_maker_address: str
    deployed_at: int
    
    # Custodial-backed specific fields
    custodian: Optional[str] = None
    proof_of_reserves_url: Optional[str] = None
    audit_frequency: Optional[str] = None
    last_audit_date: Optional[int] = None
    
    # Issuer-authorized specific fields
    issuer_address: Optional[str] = None
    cap_table_url: Optional[str] = None
    shareholder_registry_url: Optional[str] = None
    
    # Common fields
    sector: Optional[str] = None
    industry: Optional[str] = None
    description: Optional[str] = None
    is_active: bool = True
    
    def to_dict(self) -> Dict:
        """Convert to dictionary"""
        data = asdict(self)
        data['asset_type'] = self.asset_type.value
        data['settlement_type'] = self.settlement_type.value
        return data
    
    def get_disclaimer(self) -> str:
        """Get appropriate legal disclaimer"""
        if self.asset_type == AssetType.SYNTHETIC_EQUITY:
            return """
⚠️ SYNTHETIC INSTRUMENT

This token tracks the price of {symbol} but DOES NOT represent legal ownership.

• NO voting rights
• NO dividend rights (unless MM pays)
• NO redemption for real shares
• Settlement in cash/crypto only

This is a derivative for price exposure only.
""".format(symbol=self.symbol)
        
        elif self.asset_type == AssetType.CUSTODIAL_BACKED:
            return """
✅ CUSTODIAL-BACKED TOKEN

This token is backed 1:1 by real {symbol} shares held in custody.

• Legal ownership: YES
• Voting rights: {voting}
• Dividend rights: {dividend}
• Redeemable for real shares: YES

Custodian: {custodian}
Proof of Reserves: {proof}
""".format(
                symbol=self.symbol,
                voting="YES" if self.voting_rights else "NO",
                dividend="YES" if self.dividend_rights else "NO",
                custodian=self.custodian or "N/A",
                proof=self.proof_of_reserves_url or "N/A"
            )
        
        elif self.asset_type == AssetType.ISSUER_AUTHORIZED:
            return """
✅ ISSUER-AUTHORIZED TOKEN

This token is issued directly by {company} and represents real shares.

• Legal ownership: YES
• Voting rights: YES
• Dividend rights: YES
• Part of company cap table: YES

This is a legally recognized security.
""".format(company=self.company_name)
        
        return "Unknown asset type"

@dataclass
class PriceUpdate:
    """Price update record"""
    contract_id: str
    price: Decimal
    source: str
    proof_hash: str
    timestamp: int
    market_maker_address: str

@dataclass
class CorporateAction:
    """Corporate action record"""
    contract_id: str
    action_type: str  # DIVIDEND, SPLIT, BUYBACK, MERGER
    value: Decimal
    description: str
    executed_at: int
    market_maker_address: str

@dataclass
class ProofOfReserves:
    """Proof of reserves for custodial-backed tokens"""
    contract_id: str
    custodian: str
    shares_held: int
    tokens_issued: int
    audit_date: int
    auditor: str
    report_url: str
    signature: str

class StockContractManager:
    """Manages stock contracts on blockchain"""
    
    def __init__(self):
        self.contracts: Dict[str, StockContract] = {}
        self.price_history: Dict[str, List[PriceUpdate]] = {}
        self.corporate_actions: Dict[str, List[CorporateAction]] = {}
        self.proof_of_reserves: Dict[str, List[ProofOfReserves]] = {}
        self.balances: Dict[str, Dict[str, Decimal]] = {}  # address -> {contract_id -> balance}
    
    def deploy_synthetic_equity(
        self,
        symbol: str,
        company_name: str,
        exchange: str,
        token_supply: int,
        price_source: str,
        market_maker_address: str,
        sector: Optional[str] = None,
        industry: Optional[str] = None,
        description: Optional[str] = None
    ) -> StockContract:
        """Deploy synthetic (price-tracking) stock contract"""
        
        contract_id = self._generate_contract_id(symbol, market_maker_address)
        
        contract = StockContract(
            contract_id=contract_id,
            asset_type=AssetType.SYNTHETIC_EQUITY,
            symbol=symbol,
            company_name=company_name,
            exchange=exchange,
            token_supply=token_supply,
            price_source=price_source,
            legal_ownership=False,
            settlement_type=SettlementType.CASH_CRYPTO,
            voting_rights=False,
            dividend_rights=False,
            redemption_rights=False,
            market_maker_address=market_maker_address,
            deployed_at=int(time.time()),
            sector=sector,
            industry=industry,
            description=description or f"Synthetic token tracking {symbol} price"
        )
        
        self.contracts[contract_id] = contract
        self.price_history[contract_id] = []
        self.corporate_actions[contract_id] = []
        
        # Market maker gets initial supply
        if market_maker_address not in self.balances:
            self.balances[market_maker_address] = {}
        self.balances[market_maker_address][contract_id] = Decimal(token_supply)
        
        return contract
    
    def deploy_custodial_backed(
        self,
        symbol: str,
        company_name: str,
        exchange: str,
        shares_held: int,
        price_source: str,
        market_maker_address: str,
        custodian: str,
        proof_of_reserves_url: str,
        audit_frequency: str,
        voting_rights: bool = False,
        dividend_rights: bool = False,
        sector: Optional[str] = None,
        industry: Optional[str] = None
    ) -> StockContract:
        """Deploy custodial-backed (1:1) stock contract"""
        
        contract_id = self._generate_contract_id(symbol, market_maker_address)
        
        contract = StockContract(
            contract_id=contract_id,
            asset_type=AssetType.CUSTODIAL_BACKED,
            symbol=symbol,
            company_name=company_name,
            exchange=exchange,
            token_supply=shares_held,  # MUST equal shares held
            price_source=price_source,
            legal_ownership=True,
            settlement_type=SettlementType.PHYSICAL_REDEMPTION,
            voting_rights=voting_rights,
            dividend_rights=dividend_rights,
            redemption_rights=True,
            market_maker_address=market_maker_address,
            deployed_at=int(time.time()),
            custodian=custodian,
            proof_of_reserves_url=proof_of_reserves_url,
            audit_frequency=audit_frequency,
            last_audit_date=int(time.time()),
            sector=sector,
            industry=industry,
            description=f"Custodial-backed {symbol} tokens (1:1 with real shares)"
        )
        
        self.contracts[contract_id] = contract
        self.price_history[contract_id] = []
        self.corporate_actions[contract_id] = []
        self.proof_of_reserves[contract_id] = []
        
        # Market maker gets initial supply
        if market_maker_address not in self.balances:
            self.balances[market_maker_address] = {}
        self.balances[market_maker_address][contract_id] = Decimal(shares_held)
        
        return contract
    
    def deploy_issuer_authorized(
        self,
        symbol: str,
        company_name: str,
        exchange: str,
        shares_issued: int,
        issuer_address: str,
        cap_table_url: str,
        shareholder_registry_url: str,
        sector: Optional[str] = None,
        industry: Optional[str] = None
    ) -> StockContract:
        """Deploy issuer-authorized (company-issued) stock contract"""
        
        contract_id = self._generate_contract_id(symbol, issuer_address)
        
        contract = StockContract(
            contract_id=contract_id,
            asset_type=AssetType.ISSUER_AUTHORIZED,
            symbol=symbol,
            company_name=company_name,
            exchange=exchange,
            token_supply=shares_issued,
            price_source="Company Registry",
            legal_ownership=True,
            settlement_type=SettlementType.COMPANY_REGISTRY,
            voting_rights=True,
            dividend_rights=True,
            redemption_rights=False,  # Already on cap table
            market_maker_address=issuer_address,
            deployed_at=int(time.time()),
            issuer_address=issuer_address,
            cap_table_url=cap_table_url,
            shareholder_registry_url=shareholder_registry_url,
            sector=sector,
            industry=industry,
            description=f"Company-issued {symbol} shares on blockchain"
        )
        
        self.contracts[contract_id] = contract
        self.price_history[contract_id] = []
        self.corporate_actions[contract_id] = []
        
        # Issuer gets initial supply
        if issuer_address not in self.balances:
            self.balances[issuer_address] = {}
        self.balances[issuer_address][contract_id] = Decimal(shares_issued)
        
        return contract
    
    def update_price(
        self,
        contract_id: str,
        price: Decimal,
        source: str,
        proof_hash: str,
        market_maker_address: str
    ) -> PriceUpdate:
        """Update stock price"""
        
        if contract_id not in self.contracts:
            raise ValueError(f"Contract {contract_id} not found")
        
        contract = self.contracts[contract_id]
        
        # Verify market maker authorization
        if contract.market_maker_address != market_maker_address:
            raise PermissionError("Only authorized market maker can update price")
        
        price_update = PriceUpdate(
            contract_id=contract_id,
            price=price,
            source=source,
            proof_hash=proof_hash,
            timestamp=int(time.time()),
            market_maker_address=market_maker_address
        )
        
        self.price_history[contract_id].append(price_update)
        
        return price_update
    
    def execute_corporate_action(
        self,
        contract_id: str,
        action_type: str,
        value: Decimal,
        description: str,
        market_maker_address: str
    ) -> CorporateAction:
        """Execute corporate action"""
        
        if contract_id not in self.contracts:
            raise ValueError(f"Contract {contract_id} not found")
        
        contract = self.contracts[contract_id]
        
        # Verify authorization
        if contract.market_maker_address != market_maker_address:
            raise PermissionError("Only authorized market maker can execute corporate actions")
        
        action = CorporateAction(
            contract_id=contract_id,
            action_type=action_type,
            value=value,
            description=description,
            executed_at=int(time.time()),
            market_maker_address=market_maker_address
        )
        
        self.corporate_actions[contract_id].append(action)
        
        # Handle stock split
        if action_type == "SPLIT":
            split_ratio = float(value)
            # For synthetic: price adjusts automatically via price feed
            # For custodial: need to mint new tokens
            if contract.asset_type == AssetType.CUSTODIAL_BACKED:
                # Mint new tokens proportionally
                for address in self.balances:
                    if contract_id in self.balances[address]:
                        old_balance = self.balances[address][contract_id]
                        new_balance = old_balance * Decimal(split_ratio)
                        self.balances[address][contract_id] = new_balance
                
                # Update total supply
                contract.token_supply = int(contract.token_supply * split_ratio)
        
        return action
    
    def submit_proof_of_reserves(
        self,
        contract_id: str,
        custodian: str,
        shares_held: int,
        tokens_issued: int,
        auditor: str,
        report_url: str,
        signature: str
    ) -> ProofOfReserves:
        """Submit proof of reserves for custodial-backed tokens"""
        
        if contract_id not in self.contracts:
            raise ValueError(f"Contract {contract_id} not found")
        
        contract = self.contracts[contract_id]
        
        if contract.asset_type != AssetType.CUSTODIAL_BACKED:
            raise ValueError("Proof of reserves only for custodial-backed tokens")
        
        # Verify supply matches
        if shares_held != tokens_issued:
            raise ValueError(f"Shares held ({shares_held}) must equal tokens issued ({tokens_issued})")
        
        proof = ProofOfReserves(
            contract_id=contract_id,
            custodian=custodian,
            shares_held=shares_held,
            tokens_issued=tokens_issued,
            audit_date=int(time.time()),
            auditor=auditor,
            report_url=report_url,
            signature=signature
        )
        
        self.proof_of_reserves[contract_id].append(proof)
        contract.last_audit_date = proof.audit_date
        
        return proof
    
    def transfer(
        self,
        contract_id: str,
        from_address: str,
        to_address: str,
        amount: Decimal
    ) -> bool:
        """Transfer tokens"""
        
        if contract_id not in self.contracts:
            raise ValueError(f"Contract {contract_id} not found")
        
        # Check balance
        if from_address not in self.balances or contract_id not in self.balances[from_address]:
            raise ValueError("Insufficient balance")
        
        if self.balances[from_address][contract_id] < amount:
            raise ValueError("Insufficient balance")
        
        # Transfer
        self.balances[from_address][contract_id] -= amount
        
        if to_address not in self.balances:
            self.balances[to_address] = {}
        if contract_id not in self.balances[to_address]:
            self.balances[to_address][contract_id] = Decimal(0)
        
        self.balances[to_address][contract_id] += amount
        
        return True
    
    def get_balance(self, address: str, contract_id: str) -> Decimal:
        """Get token balance"""
        if address not in self.balances or contract_id not in self.balances[address]:
            return Decimal(0)
        return self.balances[address][contract_id]
    
    def get_contract(self, contract_id: str) -> Optional[StockContract]:
        """Get contract details"""
        return self.contracts.get(contract_id)
    
    def get_current_price(self, contract_id: str) -> Optional[Decimal]:
        """Get current price"""
        if contract_id not in self.price_history or not self.price_history[contract_id]:
            return None
        return self.price_history[contract_id][-1].price
    
    def get_price_history(self, contract_id: str, limit: int = 100) -> List[PriceUpdate]:
        """Get price history"""
        if contract_id not in self.price_history:
            return []
        return self.price_history[contract_id][-limit:]
    
    def get_corporate_actions(self, contract_id: str) -> List[CorporateAction]:
        """Get corporate actions"""
        return self.corporate_actions.get(contract_id, [])
    
    def get_proof_of_reserves(self, contract_id: str) -> List[ProofOfReserves]:
        """Get proof of reserves"""
        return self.proof_of_reserves.get(contract_id, [])
    
    def _generate_contract_id(self, symbol: str, address: str) -> str:
        """Generate unique contract ID"""
        data = f"{symbol}_{address}_{int(time.time())}"
        return hashlib.sha256(data.encode()).hexdigest()[:16]

# Export
__all__ = [
    'AssetType',
    'SettlementType',
    'StockContract',
    'PriceUpdate',
    'CorporateAction',
    'ProofOfReserves',
    'StockContractManager'
]
