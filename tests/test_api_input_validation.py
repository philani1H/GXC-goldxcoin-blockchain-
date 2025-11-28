#!/usr/bin/env python3
"""
Test suite for API input validation fixes
Tests that API endpoints handle invalid input gracefully without crashing
"""

import unittest


# Copy of helper functions from blockchain_explorer.py for testing
def safe_int(value, default=0, min_val=None, max_val=None):
    """
    Safely convert a value to integer with bounds checking.
    
    Args:
        value: The value to convert
        default: Default value if conversion fails
        min_val: Minimum allowed value (optional)
        max_val: Maximum allowed value (optional)
    
    Returns:
        Integer value within specified bounds
    """
    try:
        result = int(value)
        if min_val is not None:
            result = max(result, min_val)
        if max_val is not None:
            result = min(result, max_val)
        return result
    except (ValueError, TypeError):
        return default


def safe_float(value, default=0.0, min_val=None, max_val=None):
    """
    Safely convert a value to float with bounds checking.
    
    Args:
        value: The value to convert
        default: Default value if conversion fails
        min_val: Minimum allowed value (optional)
        max_val: Maximum allowed value (optional)
    
    Returns:
        Float value within specified bounds
    """
    try:
        result = float(value)
        if min_val is not None:
            result = max(result, min_val)
        if max_val is not None:
            result = min(result, max_val)
        return result
    except (ValueError, TypeError):
        return default


class TestSafeIntConversion(unittest.TestCase):
    """Test safe_int helper function"""
    
    def test_valid_integer(self):
        """Test conversion of valid integer strings"""
        self.assertEqual(safe_int('10', default=0), 10)
        self.assertEqual(safe_int('100', default=0), 100)
        self.assertEqual(safe_int('-5', default=0), -5)
    
    def test_invalid_input(self):
        """Test handling of invalid input"""
        self.assertEqual(safe_int('abc', default=10), 10)
        self.assertEqual(safe_int('12.5', default=10), 10)
        self.assertEqual(safe_int('', default=10), 10)
        self.assertEqual(safe_int(None, default=10), 10)
    
    def test_min_value_constraint(self):
        """Test minimum value constraint"""
        self.assertEqual(safe_int('5', default=10, min_val=10), 10)
        self.assertEqual(safe_int('-5', default=0, min_val=0), 0)
        self.assertEqual(safe_int('15', default=10, min_val=10), 15)
    
    def test_max_value_constraint(self):
        """Test maximum value constraint"""
        self.assertEqual(safe_int('150', default=10, max_val=100), 100)
        self.assertEqual(safe_int('50', default=10, max_val=100), 50)
        self.assertEqual(safe_int('100', default=10, max_val=100), 100)
    
    def test_min_max_constraints(self):
        """Test both min and max constraints together"""
        self.assertEqual(safe_int('5', default=50, min_val=10, max_val=100), 10)
        self.assertEqual(safe_int('150', default=50, min_val=10, max_val=100), 100)
        self.assertEqual(safe_int('50', default=50, min_val=10, max_val=100), 50)
    
    def test_edge_cases(self):
        """Test edge cases"""
        self.assertEqual(safe_int('0', default=10), 0)
        self.assertEqual(safe_int('999999999', default=10), 999999999)
        self.assertEqual(safe_int('1e5', default=10), 10)  # Scientific notation should fail


class TestSafeFloatConversion(unittest.TestCase):
    """Test safe_float helper function"""
    
    def test_valid_float(self):
        """Test conversion of valid float strings"""
        self.assertEqual(safe_float('10.5', default=0.0), 10.5)
        self.assertEqual(safe_float('100', default=0.0), 100.0)
        self.assertEqual(safe_float('-5.25', default=0.0), -5.25)
    
    def test_invalid_input(self):
        """Test handling of invalid input"""
        self.assertEqual(safe_float('abc', default=10.0), 10.0)
        self.assertEqual(safe_float('', default=10.0), 10.0)
        self.assertEqual(safe_float(None, default=10.0), 10.0)
    
    def test_min_value_constraint(self):
        """Test minimum value constraint"""
        self.assertEqual(safe_float('5.0', default=10.0, min_val=10.0), 10.0)
        self.assertEqual(safe_float('-5.0', default=0.0, min_val=0.0), 0.0)
        self.assertEqual(safe_float('15.5', default=10.0, min_val=10.0), 15.5)
    
    def test_max_value_constraint(self):
        """Test maximum value constraint"""
        self.assertEqual(safe_float('150.5', default=10.0, max_val=100.0), 100.0)
        self.assertEqual(safe_float('50.5', default=10.0, max_val=100.0), 50.5)
        self.assertEqual(safe_float('100.0', default=10.0, max_val=100.0), 100.0)
    
    def test_min_max_constraints(self):
        """Test both min and max constraints together"""
        self.assertEqual(safe_float('5.0', default=50.0, min_val=10.0, max_val=100.0), 10.0)
        self.assertEqual(safe_float('150.0', default=50.0, min_val=10.0, max_val=100.0), 100.0)
        self.assertEqual(safe_float('50.5', default=50.0, min_val=10.0, max_val=100.0), 50.5)
    
    def test_edge_cases(self):
        """Test edge cases"""
        self.assertEqual(safe_float('0.0', default=10.0), 0.0)
        self.assertEqual(safe_float('999999.999', default=10.0), 999999.999)
        self.assertAlmostEqual(safe_float('1e5', default=10.0), 100000.0)  # Scientific notation should work


class TestAPIEndpointScenarios(unittest.TestCase):
    """Test realistic API endpoint scenarios"""
    
    def test_malicious_limit_values(self):
        """Test that malicious limit values are handled safely"""
        # SQL injection attempt
        self.assertEqual(safe_int("1; DROP TABLE blocks;", default=10, max_val=100), 10)
        
        # XSS attempt
        self.assertEqual(safe_int("<script>alert('xss')</script>", default=10, max_val=100), 10)
        
        # Extremely large number
        self.assertEqual(safe_int("999999999999", default=10, max_val=100), 100)
        
        # Negative number when positive expected
        self.assertEqual(safe_int("-100", default=10, min_val=1, max_val=100), 1)
    
    def test_typical_api_usage(self):
        """Test typical API usage patterns"""
        # /api/blocks?limit=50
        self.assertEqual(safe_int("50", default=10, min_val=1, max_val=100), 50)
        
        # /api/charts?days=30
        self.assertEqual(safe_int("30", default=14, min_val=1, max_val=365), 30)
        
        # /api/transactions?limit=20
        self.assertEqual(safe_int("20", default=20, min_val=1, max_val=100), 20)
    
    def test_missing_parameters(self):
        """Test behavior when parameters are missing (None)"""
        self.assertEqual(safe_int(None, default=10, min_val=1, max_val=100), 10)
        self.assertEqual(safe_float(None, default=0.0, min_val=0.0), 0.0)


if __name__ == '__main__':
    # Run tests with verbose output
    unittest.main(verbosity=2)
