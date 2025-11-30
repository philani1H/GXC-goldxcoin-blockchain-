#!/usr/bin/env python3
"""
Comprehensive test to verify all URLs return data
Uses static code analysis - no server required
"""

import os
import re
import ast
from pathlib import Path

print("="*70)
print("GXC EXPLORER - URL DATA RETURN VERIFICATION")
print("="*70)

# Results
results = {
    'endpoints_with_data': [],
    'endpoints_missing_data': [],
    'api_endpoints': [],
    'wallet_endpoints': [],
    'data_sources': {
        'database_queries': 0,
        'rpc_calls': 0,
        'jsonify_calls': 0,
        'render_template_calls': 0
    }
}

def analyze_file(file_path, file_type='explorer'):
    """Analyze a Python file for data return patterns"""
    if not os.path.exists(file_path):
        return
    
    print(f"\n{'='*70}")
    print(f"Analyzing: {file_path}")
    print(f"{'='*70}\n")
    
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Find all routes
        route_pattern = r"@app\.route\(['\"]([^'\"]+)['\"](?:,\s*methods=\[([^\]]+)\])?\)"
        routes = []
        for match in re.finditer(route_pattern, content):
            route_path = match.group(1)
            methods = match.group(2) if match.group(2) else 'GET'
            methods = [m.strip().strip("'\"") for m in methods.split(',')] if methods else ['GET']
            
            # Find function name
            func_pattern = rf"@app\.route\(['\"]{re.escape(route_path)}['\"].*?\ndef\s+(\w+)\("
            func_match = re.search(func_pattern, content, re.DOTALL)
            func_name = func_match.group(1) if func_match else 'unknown'
            
            routes.append({
                'path': route_path,
                'methods': methods,
                'function': func_name
            })
        
        # Analyze each route
        for route in routes:
            func_name = route['function']
            path = route['path']
            
            # Find function body
            func_pattern = rf"def\s+{re.escape(func_name)}\(.*?\):(.*?)(?=def\s+\w+\(|@app\.route|if\s+__name__|$)"
            func_match = re.search(func_pattern, content, re.DOTALL)
            
            if func_match:
                func_body = func_match.group(1)
                
                # Check for data return
                has_data = False
                data_sources = []
                
                # Check for jsonify (API endpoint)
                if 'jsonify' in func_body:
                    results['data_sources']['jsonify_calls'] += 1
                    has_data = True
                    data_sources.append('jsonify')
                    
                    # Check what data is being returned
                    jsonify_pattern = r"jsonify\(([^)]+)\)"
                    jsonify_matches = re.findall(jsonify_pattern, func_body)
                    for match in jsonify_matches:
                        if 'explorer.' in match or 'get_' in match or 'cursor.' in match:
                            data_sources.append('database/rpc')
                
                # Check for render_template (page endpoint)
                if 'render_template' in func_body:
                    results['data_sources']['render_template_calls'] += 1
                    has_data = True
                    data_sources.append('template')
                    
                    # Check if data is passed to template
                    template_pattern = r"render_template\(['\"]([^'\"]+)['\"],\s*([^)]+)\)"
                    template_matches = re.findall(template_pattern, func_body)
                    for template_name, template_data in template_matches:
                        if 'explorer.' in template_data or 'get_' in template_data or 'network_info' in template_data:
                            data_sources.append('data_passed')
                
                # Check for database queries
                if 'cursor.execute' in func_body or 'SELECT' in func_body.upper():
                    results['data_sources']['database_queries'] += 1
                    has_data = True
                    data_sources.append('database')
                
                # Check for RPC calls
                if 'rpc_call' in func_body:
                    results['data_sources']['rpc_calls'] += 1
                    has_data = True
                    data_sources.append('rpc')
                
                # Categorize
                if path.startswith('/api'):
                    results['api_endpoints'].append({
                        'path': path,
                        'function': func_name,
                        'has_data': has_data,
                        'sources': data_sources
                    })
                elif 'wallet' in path.lower() or file_type == 'wallet':
                    results['wallet_endpoints'].append({
                        'path': path,
                        'function': func_name,
                        'has_data': has_data,
                        'sources': data_sources
                    })
                else:
                    if has_data:
                        results['endpoints_with_data'].append({
                            'path': path,
                            'function': func_name,
                            'sources': data_sources
                        })
                    else:
                        results['endpoints_missing_data'].append({
                            'path': path,
                            'function': func_name
                        })
                
                # Print status
                status = "✅" if has_data else "⚠️"
                sources_str = ", ".join(data_sources) if data_sources else "no data"
                print(f"{status} {path:50} -> {func_name:30} [{sources_str}]")
    
    except Exception as e:
        print(f"Error analyzing {file_path}: {e}")

def print_summary():
    """Print comprehensive summary"""
    print(f"\n{'='*70}")
    print("VERIFICATION SUMMARY")
    print(f"{'='*70}\n")
    
    total_endpoints = (
        len(results['endpoints_with_data']) +
        len(results['endpoints_missing_data']) +
        len(results['api_endpoints']) +
        len(results['wallet_endpoints'])
    )
    
    endpoints_with_data = (
        len(results['endpoints_with_data']) +
        len([e for e in results['api_endpoints'] if e['has_data']]) +
        len([e for e in results['wallet_endpoints'] if e['has_data']])
    )
    
    print(f"📊 Total Endpoints Analyzed: {total_endpoints}")
    print(f"✅ Endpoints with Data: {endpoints_with_data}")
    print(f"⚠️  Endpoints Missing Data: {len(results['endpoints_missing_data'])}")
    print(f"🔌 API Endpoints: {len(results['api_endpoints'])}")
    print(f"💼 Wallet Endpoints: {len(results['wallet_endpoints'])}")
    
    print(f"\n📈 Data Sources:")
    print(f"   - Database Queries: {results['data_sources']['database_queries']}")
    print(f"   - RPC Calls: {results['data_sources']['rpc_calls']}")
    print(f"   - JSON Responses: {results['data_sources']['jsonify_calls']}")
    print(f"   - Template Renders: {results['data_sources']['render_template_calls']}")
    
    if results['endpoints_missing_data']:
        print(f"\n⚠️  Endpoints That May Not Return Data:")
        for endpoint in results['endpoints_missing_data']:
            print(f"   - {endpoint['path']} ({endpoint['function']})")
    
    # Save detailed report
    with open('URL_DATA_RETURN_REPORT.md', 'w') as f:
        f.write("# URL Data Return Verification Report\n\n")
        f.write(f"## Summary\n\n")
        f.write(f"- Total Endpoints: {total_endpoints}\n")
        f.write(f"- Endpoints with Data: {endpoints_with_data}\n")
        f.write(f"- Endpoints Missing Data: {len(results['endpoints_missing_data'])}\n")
        f.write(f"- API Endpoints: {len(results['api_endpoints'])}\n")
        f.write(f"- Wallet Endpoints: {len(results['wallet_endpoints'])}\n\n")
        
        f.write(f"## Data Sources\n\n")
        f.write(f"- Database Queries: {results['data_sources']['database_queries']}\n")
        f.write(f"- RPC Calls: {results['data_sources']['rpc_calls']}\n")
        f.write(f"- JSON Responses: {results['data_sources']['jsonify_calls']}\n")
        f.write(f"- Template Renders: {results['data_sources']['render_template_calls']}\n\n")
        
        f.write(f"## Endpoints with Data\n\n")
        for endpoint in results['endpoints_with_data']:
            f.write(f"- `{endpoint['path']}` -> `{endpoint['function']}()`\n")
            f.write(f"  - Data sources: {', '.join(endpoint['sources'])}\n")
        
        f.write(f"\n## API Endpoints\n\n")
        for endpoint in results['api_endpoints']:
            status = "✅" if endpoint['has_data'] else "⚠️"
            f.write(f"- {status} `{endpoint['path']}` -> `{endpoint['function']}()`\n")
            if endpoint['sources']:
                f.write(f"  - Data sources: {', '.join(endpoint['sources'])}\n")
        
        f.write(f"\n## Wallet Endpoints\n\n")
        for endpoint in results['wallet_endpoints']:
            status = "✅" if endpoint['has_data'] else "⚠️"
            f.write(f"- {status} `{endpoint['path']}` -> `{endpoint['function']}()`\n")
            if endpoint['sources']:
                f.write(f"  - Data sources: {', '.join(endpoint['sources'])}\n")
        
        if results['endpoints_missing_data']:
            f.write(f"\n## Endpoints Missing Data\n\n")
            for endpoint in results['endpoints_missing_data']:
                f.write(f"- `{endpoint['path']}` -> `{endpoint['function']}()`\n")
    
    print(f"\n✅ Detailed report saved to: URL_DATA_RETURN_REPORT.md")
    
    # Final verdict
    success_rate = (endpoints_with_data / total_endpoints * 100) if total_endpoints > 0 else 0
    print(f"\n{'='*70}")
    print(f"✅ Success Rate: {success_rate:.1f}% ({endpoints_with_data}/{total_endpoints} endpoints return data)")
    print(f"{'='*70}\n")
    
    if success_rate >= 95:
        print("🎉 EXCELLENT: Almost all endpoints return data!")
    elif success_rate >= 80:
        print("✅ GOOD: Most endpoints return data")
    else:
        print("⚠️  WARNING: Some endpoints may not return data")

if __name__ == '__main__':
    # Analyze explorer
    analyze_file('web/blockchain_explorer.py', 'explorer')
    
    # Analyze wallet service
    analyze_file('api/wallet_service.py', 'wallet')
    
    # Print summary
    print_summary()
