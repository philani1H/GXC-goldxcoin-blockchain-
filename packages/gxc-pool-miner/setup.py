#!/usr/bin/env python3
"""
GXC Pool Miner - Setup Script
Creates installable package
"""

from setuptools import setup, find_packages
import os
import sys

# Read version from source
VERSION = "2.0.0"

# Read README
readme_path = os.path.join(os.path.dirname(__file__), 'README.md')
long_description = ""
if os.path.exists(readme_path):
    with open(readme_path, 'r', encoding='utf-8') as f:
        long_description = f.read()

# Platform-specific dependencies
install_requires = [
    'customtkinter>=5.2.0',
    'psutil>=5.9.0',
    'requests>=2.28.0',
    'matplotlib>=3.7.0',
]

# Entry points
entry_points = {
    'console_scripts': [
        'gxc-pool-miner=src.gxc_pool_miner:main',
    ],
    'gui_scripts': [
        'gxc-pool-miner-gui=src.gxc_pool_miner:main',
    ],
}

setup(
    name='gxc-pool-miner',
    version=VERSION,
    author='GXC Blockchain',
    author_email='dev@gxc.network',
    description='Universal Mining Management System for GXC Blockchain',
    long_description=long_description,
    long_description_content_type='text/markdown',
    url='https://github.com/gxc-blockchain/gxc-pool-miner',
    project_urls={
        'Documentation': 'https://docs.gxc.io/mining',
        'Source': 'https://github.com/gxc-blockchain/gxc-pool-miner',
        'Tracker': 'https://github.com/gxc-blockchain/gxc-pool-miner/issues',
    },
    packages=find_packages(),
    package_data={
        '': ['assets/*', 'LICENSE', 'README.md'],
    },
    include_package_data=True,
    install_requires=install_requires,
    entry_points=entry_points,
    python_requires='>=3.8',
    classifiers=[
        'Development Status :: 4 - Beta',
        'Environment :: X11 Applications',
        'Environment :: Win32 (MS Windows)',
        'Environment :: MacOS X',
        'Intended Audience :: End Users/Desktop',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: MIT License',
        'Operating System :: OS Independent',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Programming Language :: Python :: 3.10',
        'Programming Language :: Python :: 3.11',
        'Programming Language :: Python :: 3.12',
        'Topic :: Internet',
        'Topic :: Security :: Cryptography',
        'Topic :: System :: Hardware',
    ],
    keywords='gxc blockchain mining pool cryptocurrency sha256 ethash',
)
