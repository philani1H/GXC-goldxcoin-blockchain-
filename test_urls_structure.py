#!/usr/bin/env python3
"""
Test URL structure and verify all endpoints are properly defined
Uses static code analysis to verify endpoints without running server
"""

import os
import re
import ast
from pathlib import Path

# Test results
results = {
    'endpoints_found': [],
    'wallet_endpoints': [],
    'api_endpoints': [],
    'missing_templates': [],
    'missing_network_info': []
}

def find_all_routes(file_path):
    """Find all Flask routes in a file"""
    routes = []
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
            
        # Find all @app.route decorators
        route_pattern = r"@app\.route\(['\"]([^'\"]+)['\"](?:,\s*methods=\[([^\]]+)\])?\)"
        matches = re.finditer(route_pattern, content)
        
        for match in matches:
            route_path = match.group(1)
            methods = match.group(2) if match.group(2) else 'GET'
            methods = [m.strip().strip("'\"") for m in methods.split(',')] if methods else ['GET']
            
            # Find the function name after the decorator
            func_pattern = rf"@app\.route\(['\"]{re.escape(route_path)}['\"].*?\ndef\s+(\w+)\("
            func_match = re.search(func_pattern, content, re.DOTALL)
            func_name = func_match.group(1) if func_match else 'unknown'
            
            routes.append({
                'path': route_path,
                'methods': methods,
                'function': func_name,
                'file': file_path
            })
    except Exception as e:
        print(f"Error reading {file_path}: {e}")
    
    return routes

def check_template_exists(template_name, template_dir='web/templates'):
    """Check if a template file exists"""
    template_path = Path(template_dir) / template_name
    return template_path.exists()

def check_network_info_in_route(file_path, route_func_name):
    """Check if route includes network_info in render_template"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Find the function
        func_pattern = rf"def\s+{re.escape(route_func_name)}\(.*?\):"
        func_match = re.search(func_pattern, content, re.DOTALL)
        
        if func_match:
            func_body = func_match.group(0)
            
            # Check for render_template calls
            if 'render_template' in func_body:
                # Check if network_info is passed
                if 'network_info' in func_body:
                    return True
                else:
                    # Check if it's an API endpoint (jsonify)
                    if 'jsonify' in func_body:
                        return True  # API endpoints don't need network_info
                    return False
        return None
    except Exception as e:
        print(f"Error checking network_info: {e}")
        return None

def analyze_explorer_file():
    """Analyze blockchain_explorer.py"""
    file_path = 'web/blockchain_explorer.py'
    
    if not os.path.exists(file_path):
        print(f"❌ {file_path} not found")
        return
    
    print(f"\n{'='*60}")
    print(f"Analyzing: {file_path}")
    print(f"{'='*60}\n")
    
    routes = find_all_routes(file_path)
    
    print(f"Found {len(routes)} routes:\n")
    
    for route in routes:
        path = route['path']
        methods = route['methods']
        func_name = route['function']
        
        # Categorize routes
        if path.startswith('/api'):
            results['api_endpoints'].append(route)
            category = "API"
        elif 'wallet' in path.lower():
            results['wallet_endpoints'].append(route)
            category = "WALLET"
        else:
            results['endpoints_found'].append(route)
            category = "PAGE"
        
        # Check if it renders a template
        if 'render_template' in open(file_path).read():
            # Try to find template name
            template_match = re.search(
                rf"def\s+{re.escape(func_name)}.*?render_template\(['\"]([^'\"]+)['\"]",
                open(file_path).read(),
                re.DOTALL
            )
            if template_match:
                template_name = template_match.group(1)
                if not check_template_exists(template_name):
                    results['missing_templates'].append({
                        'route': path,
                        'template': template_name
                    })
        
        # Check network_info
        has_network_info = check_network_info_in_route(file_path, func_name)
        if has_network_info is False and not path.startswith('/api'):
            results['missing_network_info'].append({
                'route': path,
                'function': func_name
            })
        
        status = "✅" if has_network_info or path.startswith('/api') else "⚠️"
        print(f"{status} {category:6} {methods[0]:4} {path:40} -> {func_name}()")
    
    return routes

def analyze_wallet_file():
    """Analyze wallet_service.py"""
    file_path = 'api/wallet_service.py'
    
    if not os.path.exists(file_path):
        print(f"\n⚠️  {file_path} not found (wallet service may be separate)")
        return
    
    print(f"\n{'='*60}")
    print(f"Analyzing: {file_path}")
    print(f"{'='*60}\n")
    
    routes = find_all_routes(file_path)
    
    print(f"Found {len(routes)} wallet routes:\n")
    
    for route in routes:
        path = route['path']
        methods = route['methods']
        func_name = route['function']
        
        results['wallet_endpoints'].append(route)
        
        print(f"✅ WALLET {methods[0]:4} {path:50} -> {func_name}()")
    
    return routes

def print_summary():
    """Print test summary"""
    print(f"\n{'='*60}")
    print("TEST SUMMARY")
    print(f"{'='*60}\n")
    
    print(f"📄 Page Endpoints: {len(results['endpoints_found'])}")
    print(f"🔌 API Endpoints: {len(results['api_endpoints'])}")
    print(f"💼 Wallet Endpoints: {len(results['wallet_endpoints'])}")
    print(f"📝 Total Endpoints: {len(results['endpoints_found']) + len(results['api_endpoints']) + len(results['wallet_endpoints'])}")
    
    if results['missing_templates']:
        print(f"\n⚠️  Missing Templates: {len(results['missing_templates'])}")
        for item in results['missing_templates']:
            print(f"   - {item['route']} -> {item['template']}")
    
    if results['missing_network_info']:
        print(f"\n⚠️  Missing network_info: {len(results['missing_network_info'])}")
        for item in results['missing_network_info']:
            print(f"   - {item['route']} ({item['function']})")
    
    # Save detailed report
    with open('URL_STRUCTURE_REPORT.md', 'w') as f:
        f.write("# URL Structure Report\n\n")
        f.write("## Page Endpoints\n\n")
        for route in results['endpoints_found']:
            f.write(f"- `{route['methods'][0]} {route['path']}` -> `{route['function']}()`\n")
        
        f.write("\n## API Endpoints\n\n")
        for route in results['api_endpoints']:
            f.write(f"- `{route['methods'][0]} {route['path']}` -> `{route['function']}()`\n")
        
        f.write("\n## Wallet Endpoints\n\n")
        for route in results['wallet_endpoints']:
            f.write(f"- `{route['methods'][0]} {route['path']}` -> `{route['function']}()`\n")
        
        if results['missing_templates']:
            f.write("\n## Missing Templates\n\n")
            for item in results['missing_templates']:
                f.write(f"- `{item['route']}` requires template: `{item['template']}`\n")
        
        if results['missing_network_info']:
            f.write("\n## Routes Missing network_info\n\n")
            for item in results['missing_network_info']:
                f.write(f"- `{item['route']}` in function `{item['function']}()`\n")
    
    print(f"\n✅ Detailed report saved to: URL_STRUCTURE_REPORT.md")

def verify_data_return():
    """Verify that endpoints return data"""
    print(f"\n{'='*60}")
    print("VERIFYING DATA RETURN PATTERNS")
    print(f"{'='*60}\n")
    
    file_path = 'web/blockchain_explorer.py'
    
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Check for jsonify patterns
        jsonify_pattern = r"jsonify\(({[^}]+})\)"
        jsonify_matches = re.findall(jsonify_pattern, content)
        
        print(f"Found {len(jsonify_matches)} jsonify calls (API endpoints returning data)")
        
        # Check for render_template with data
        template_pattern = r"render_template\(['\"]([^'\"]+)['\"],\s*({[^}]+})\)"
        template_matches = re.findall(template_pattern, content)
        
        print(f"Found {len(template_matches)} render_template calls with data")
        
        # Check for database queries
        db_query_pattern = r"cursor\.execute\(['\"](SELECT[^'\"]+)['\"]"
        db_queries = re.findall(db_query_pattern, content, re.IGNORECASE)
        
        print(f"Found {len(db_queries)} database SELECT queries")
        
        # Check for RPC calls
        rpc_pattern = r"rpc_call\(['\"]([^'\"]+)['\"]"
        rpc_calls = re.findall(rpc_pattern, content)
        
        print(f"Found {len(rpc_calls)} RPC calls to blockchain node")
        
    except Exception as e:
        print(f"Error verifying data return: {e}")

if __name__ == '__main__':
    print("="*60)
    print("GXC EXPLORER URL STRUCTURE TEST")
    print("="*60)
    
    # Analyze explorer
    explorer_routes = analyze_explorer_file()
    
    # Analyze wallet service
    wallet_routes = analyze_wallet_file()
    
    # Verify data return patterns
    verify_data_return()
    
    # Print summary
    print_summary()
    
    print(f"\n{'='*60}")
    print("✅ URL structure analysis complete!")
    print(f"{'='*60}\n")
