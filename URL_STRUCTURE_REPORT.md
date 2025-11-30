# URL Structure Report

## Page Endpoints

- `GET /` -> `index()`
- `GET /block/<block_number>` -> `block_detail()`
- `GET /tx/<tx_hash>` -> `transaction_detail()`
- `GET /address/<address>` -> `address_detail()`
- `GET /search` -> `search()`
- `GET /blocks` -> `blocks_list()`
- `GET /transactions` -> `transactions_list()`
- `GET /validators` -> `validators_list()`
- `GET /addresses` -> `addresses_list()`
- `GET /mining` -> `mining_page()`
- `GET /mining/guide` -> `mining_guide()`
- `GET /stocks` -> `stocks_page()`
- `GET /stocks/guide` -> `stocks_guide()`
- `GET /forum` -> `forum_redirect()`
- `GET /stocks/<ticker>` -> `stock_detail()`
- `GET /charts` -> `charts_page()`
- `GET /gas` -> `gas_tracker()`
- `GET /price` -> `price_page()`
- `GET /portfolio` -> `portfolio_page()`
- `GET /health` -> `network_health()`
- `GET /healthz` -> `health_check()`
- `GET /healthcheck` -> `health_check()`
- `GET /decoder` -> `transaction_decoder()`
- `GET /analytics` -> `analytics_page()`
- `GET /pending` -> `pending_transactions()`
- `GET /gold` -> `gold_token_explorer()`
- `GET /governance` -> `governance_dashboard()`
- `GET /bridge` -> `bridge_explorer()`
- `GET /staking` -> `staking_dashboard()`
- `GET /graph` -> `transaction_graph()`
- `GET /alerts` -> `price_alerts()`
- `GET /simulator` -> `transaction_simulator()`
- `GET /webhooks` -> `webhooks_page()`
- `GET /downloads` -> `downloads_page()`

## API Endpoints

- `GET /api` -> `api_docs()`
- `GET /api/blocks` -> `api_blocks()`
- `GET /api/transactions` -> `api_transactions()`
- `GET /api/stats` -> `api_stats()`
- `GET /api/charts` -> `api_charts()`
- `GET /api/hashrate` -> `api_hashrate()`
- `GET /api/gas` -> `api_gas()`
- `GET /api/price` -> `api_price()`
- `GET /api/portfolio` -> `api_portfolio()`
- `GET /api/health` -> `api_health()`
- `GET /api/connection-status` -> `api_connection_status()`
- `GET /api/network-info` -> `api_network_info()`
- `GET /api/decode` -> `api_decode()`
- `GET /api/analytics` -> `api_analytics()`
- `GET /api/pending` -> `api_pending()`
- `GET /api/export/transactions` -> `export_transactions()`
- `GET /api/export/blocks` -> `export_blocks()`
- `GET /api/export/utxo` -> `export_utxo()`
- `GET /api/export/all` -> `export_all()`
- `GET /api/filter/transactions` -> `api_filter_transactions()`
- `GET /api/gold/stats` -> `api_gold_stats()`
- `GET /api/gold/reserves` -> `api_gold_reserves()`
- `GET /api/gold/transfers` -> `api_gold_transfers()`
- `GET /api/gold/address/<address>` -> `api_gold_address()`
- `GET /api/governance/proposals` -> `api_governance_proposals()`
- `GET /api/governance/proposal/<proposal_id>` -> `api_governance_proposal()`
- `GET /api/governance/proposal/<proposal_id>/votes` -> `api_proposal_votes()`
- `GET /api/bridge/transfers` -> `api_bridge_transfers()`
- `GET /api/bridge/validators` -> `api_bridge_validators()`
- `GET /api/bridge/stats` -> `api_bridge_stats()`
- `GET /api/staking/validators` -> `api_staking_validators()`
- `GET /api/staking/validator/<address>` -> `api_staking_validator()`
- `GET /api/staking/stats` -> `api_staking_stats()`
- `GET /api/graph/address/<address>` -> `api_address_graph()`
- `GET /api/graph/transaction/<tx_hash>` -> `api_transaction_graph()`
- `GET /api/alerts` -> `api_alerts()`
- `GET /api/simulator/estimate` -> `api_simulator_estimate()`
- `GET /api/keys` -> `api_keys_page()`
- `POST /api/keys/generate` -> `api_generate_key()`
- `POST /api/keys/validate` -> `api_validate_key()`
- `GET /api/webhooks` -> `api_list_webhooks()`
- `POST /api/webhooks` -> `api_list_webhooks()`
- `DELETE /api/webhooks/<int:webhook_id>` -> `api_delete_webhook()`
- `GET /api/downloads/<platform>/<algorithm>` -> `download_miner()`

## Wallet Endpoints

- `GET /wallet` -> `wallet_page()`
- `GET /` -> `index()`
- `GET /register` -> `register_page()`
- `GET /login` -> `login_page()`
- `GET /logout` -> `logout()`
- `GET /create-wallet` -> `create_wallet_page()`
- `GET /wallet-success` -> `wallet_success()`
- `GET /dashboard` -> `dashboard()`
- `GET /wallet/<wallet_id>/send` -> `send_page()`
- `GET /wallet/<wallet_id>/receive` -> `receive_page()`
- `GET /wallet/<wallet_id>/history` -> `wallet_history()`
- `GET /wallet/<wallet_id>/settings` -> `wallet_settings()`
- `GET /wallet/<wallet_id>/staking` -> `staking_page()`
- `GET /wallet/<wallet_id>/become-validator` -> `become_validator_page()`
- `GET /recovery` -> `recovery_page()`
- `POST /api/v1/register` -> `register()`
- `POST /api/v1/login` -> `login()`
- `POST /api/v1/wallets/create` -> `create_wallet()`
- `GET /api/v1/wallets` -> `get_wallets()`
- `GET /api/v1/wallets/<wallet_id>/balance` -> `get_wallet_balance_api()`
- `GET /api/v1/wallets/<wallet_id>/transactions` -> `get_wallet_transactions_api()`
- `POST /api/v1/wallets/<wallet_id>/become-validator` -> `become_validator_api()`
- `GET /api/v1/wallets/<wallet_id>/validator-status` -> `get_validator_status_api()`
- `POST /api/v1/wallets/<wallet_id>/send` -> `send_transaction_api()`
- `POST /api/v1/wallets/<wallet_id>/backup` -> `store_wallet_backup()`
- `GET /api/v1/health` -> `health_check()`
- `GET /api/v1/docs` -> `api_docs()`
- `GET /docs` -> `api_docs()`
- `GET /api` -> `api_index()`

## Missing Templates

- `/forum` requires template: `stock_detail.html`
- `/stocks/<ticker>` requires template: `stock_detail.html`
