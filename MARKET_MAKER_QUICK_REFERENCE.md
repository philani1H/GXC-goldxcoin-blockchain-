# Market Maker Quick Reference Guide

## Choose Your Asset Model

**Quick Decision Tree**:

```
Do you have real shares in custody?
├─ NO → Use Model 1: Synthetic Equity ✅ (Start here)
└─ YES → Do you want to offer redemption?
    ├─ YES → Use Model 2: Custodial-Backed 💎 (Premium)
    └─ NO → Use Model 1: Synthetic Equity ✅
    
Are you the company issuing shares?
└─ YES → Use Model 3: Issuer-Authorized 🏢 (Future)
```

---

## Model 1: Synthetic Equity (RECOMMENDED)

### Quick Deploy

```bash
curl -X POST https://api.gxc.io/v1/stock/deploy/synthetic \
  -H "Authorization: Bearer YOUR_MM_API_KEY" \
  -H "Content-Type: application/json" \
  -d '{
    "market_maker_address": "YOUR_ADDRESS",
    "ticker": "AAPL",
    "company_name": "Apple Inc.",
    "exchange": "NASDAQ",
    "token_supply": 1000000,
    "price_source": "Bloomberg Terminal"
  }'
```

### What You Get
- ✅ Deploy in 1 API call
- ✅ No custody required
- ✅ Arbitrary token supply
- ✅ Lowest capital requirements
- ✅ Easiest to manage

### What Users Get
- ✅ Price exposure to AAPL
- ✅ 24/7 trading
- ✅ Instant settlement
- ❌ NO legal ownership
- ❌ NO voting rights

### Ongoing Requirements
- Update price regularly (every 1-5 minutes)
- Provide liquidity
- Settle trades in cash/crypto

---

## Model 2: Custodial-Backed (PREMIUM)

### Quick Deploy

```bash
curl -X POST https://api.gxc.io/v1/stock/deploy/custodial \
  -H "Authorization: Bearer YOUR_MM_API_KEY" \
  -H "Content-Type: application/json" \
  -d '{
    "market_maker_address": "YOUR_ADDRESS",
    "ticker": "AAPL",
    "company_name": "Apple Inc.",
    "exchange": "NASDAQ",
    "shares_held": 100000,
    "custodian": "Goldman Sachs Custody",
    "custodian_license": "CUST-12345-US",
    "proof_of_reserves_url": "https://proof.gs.com/aapl",
    "audit_frequency": "monthly"
  }'
```

### What You Need
- ⚠️ Licensed custodian
- ⚠️ Real shares purchased and held
- ⚠️ Monthly audits
- ⚠️ Proof of reserves system
- ⚠️ Higher capital requirements

### What Users Get
- ✅ Legal ownership of real shares
- ✅ Can redeem for real shares
- ✅ 24/7 trading
- ✅ Instant settlement
- ✅ Premium product

### Ongoing Requirements
- Update price regularly
- Submit proof of reserves monthly
- Maintain 1:1 backing (shares = tokens)
- Handle redemption requests

---

## Model 3: Issuer-Authorized (FUTURE)

### Quick Deploy

```bash
curl -X POST https://api.gxc.io/v1/stock/deploy/issuer \
  -H "Authorization: Bearer COMPANY_API_KEY" \
  -H "Content-Type: application/json" \
  -d '{
    "issuer_address": "COMPANY_ADDRESS",
    "ticker": "AAPL",
    "company_name": "Apple Inc.",
    "exchange": "NASDAQ",
    "shares_issued": 1000000,
    "cap_table_url": "https://apple.com/cap-table",
    "shareholder_registry_url": "https://apple.com/registry",
    "regulatory_approval": "SEC-2025-12345"
  }'
```

### What You Need
- ⚠️ Company authorization
- ⚠️ SEC/regulatory approval
- ⚠️ Legal framework
- ⚠️ Cap table integration

### What Users Get
- ✅ Legal ownership
- ✅ Voting rights
- ✅ Dividend rights
- ✅ On company cap table

---

## Common Operations

### Update Price (All Models)

```bash
curl -X POST https://api.gxc.io/v1/stock/update-price \
  -H "Authorization: Bearer YOUR_MM_API_KEY" \
  -d '{
    "market_maker_address": "YOUR_ADDRESS",
    "ticker": "AAPL",
    "price": 150.50,
    "price_source": "Bloomberg Terminal",
    "proof_hash": "0xproof..."
  }'
```

### Add Liquidity (All Models)

```bash
curl -X POST https://api.gxc.io/v1/stock/add-liquidity \
  -H "Authorization: Bearer YOUR_MM_API_KEY" \
  -d '{
    "market_maker_address": "YOUR_ADDRESS",
    "ticker": "AAPL",
    "shares": 10000,
    "bid_price": 150.25,
    "ask_price": 150.75
  }'
```

### Submit Proof of Reserves (Custodial Only)

```bash
curl -X POST https://api.gxc.io/v1/stock/proof-of-reserves \
  -H "Authorization: Bearer CUSTODIAN_API_KEY" \
  -d '{
    "contract_address": "0xabc...",
    "custodian": "Goldman Sachs Custody",
    "shares_held": 100000,
    "tokens_issued": 100000,
    "auditor": "Deloitte",
    "audit_report_url": "https://audit.deloitte.com/report",
    "signature": "0xsig..."
  }'
```

---

## Comparison Table

| Feature | Synthetic | Custodial-Backed | Issuer-Authorized |
|---------|-----------|------------------|-------------------|
| **Deploy Time** | 1 minute | 1 day | 1 month |
| **Capital Required** | Low | High | Very High |
| **Custody Required** | ❌ No | ✅ Yes | ✅ Yes |
| **Legal Ownership** | ❌ No | ✅ Yes | ✅ Yes |
| **Voting Rights** | ❌ No | ⚠️ Optional | ✅ Yes |
| **Redemption** | ❌ No | ✅ Yes | ❌ No (already on cap table) |
| **Token Supply** | Arbitrary | = Shares Held | = Shares Issued |
| **Audits Required** | ❌ No | ✅ Monthly | ✅ Quarterly |
| **Regulatory Burden** | Low | Medium | High |
| **User Pricing** | Standard | Premium | Premium+ |

---

## Revenue Models

### Synthetic Equity
- **Spread**: 0.5-1% bid-ask spread
- **Volume**: High (retail + institutions)
- **Capital**: Low
- **ROI**: 20-30% annually

### Custodial-Backed
- **Spread**: 0.3-0.5% bid-ask spread
- **Premium Fee**: 0.5% annual custody fee
- **Volume**: Medium (institutions only)
- **Capital**: High
- **ROI**: 15-25% annually

### Issuer-Authorized
- **Spread**: 0.2-0.3% bid-ask spread
- **Listing Fee**: $100k-$1M one-time
- **Volume**: High (all investors)
- **Capital**: Very High
- **ROI**: 10-20% annually

---

## Best Practices

### For Synthetic Equity

1. **Price Updates**: Every 1-5 minutes during market hours
2. **Spread Management**: Adjust based on volatility
3. **Liquidity**: Maintain 10-20% of supply as liquidity
4. **Risk Management**: Use circuit breakers
5. **Transparency**: Publish price sources

### For Custodial-Backed

1. **Proof of Reserves**: Submit monthly, on time
2. **Audit Trail**: Maintain complete records
3. **Redemption Process**: Clear, fast (T+2)
4. **Custodian Relationship**: Strong, reliable
5. **Insurance**: Consider custody insurance

### For Issuer-Authorized

1. **Regulatory Compliance**: Full SEC compliance
2. **Cap Table Integration**: Real-time sync
3. **Shareholder Communications**: Regular updates
4. **Voting Mechanism**: Blockchain-based voting
5. **Dividend Distribution**: Automated on-chain

---

## Common Mistakes to Avoid

### ❌ DON'T

1. **Deploy custodial without custodian** - Will fail verification
2. **Mismatch shares and tokens** - Trading will be halted
3. **Miss proof of reserves deadline** - Trading will be halted
4. **Use wrong asset type** - Can't change after deployment
5. **Claim legal ownership for synthetic** - Legal issues

### ✅ DO

1. **Start with synthetic** - Easiest to learn
2. **Update prices regularly** - Users expect fresh data
3. **Maintain liquidity** - Users need to trade
4. **Document everything** - Audit trail is critical
5. **Be honest about asset type** - Users appreciate transparency

---

## Support

### Documentation
- **Full API Reference**: STOCK_MARKET_API_REFERENCE.md
- **Asset Models Guide**: SYNTHETIC_STOCK_MODEL.md
- **User Guide**: WHAT_ARE_GXC_STOCK_TOKENS.md

### Contact
- **Email**: marketmakers@gxc.io
- **Telegram**: @GXC_MarketMakers
- **Discord**: discord.gg/gxc

### API Keys
- **Request**: https://gxc.io/market-maker-registration
- **Verification**: 1-3 business days
- **Cost**: Free (revenue share model)

---

## Quick Start Checklist

### Day 1: Registration
- [ ] Register as market maker
- [ ] Submit license verification
- [ ] Receive API key
- [ ] Test API in sandbox

### Day 2: First Deployment
- [ ] Choose asset model (start with synthetic)
- [ ] Deploy first stock (AAPL recommended)
- [ ] Set initial price
- [ ] Add liquidity

### Day 3: Operations
- [ ] Set up automated price updates
- [ ] Monitor trading volume
- [ ] Adjust spreads based on volatility
- [ ] Review first day metrics

### Week 1: Scale
- [ ] Deploy 5-10 more stocks
- [ ] Optimize spread strategy
- [ ] Implement risk management
- [ ] Consider custodial-backed for premium tier

---

**🚀 Ready to Start? Deploy Your First Stock in 5 Minutes! 🚀**

**Version**: 3.0  
**Date**: December 22, 2025  
**Status**: Production Ready
