#!/usr/bin/env python3
"""
GXC Miner Pro - Setup Script
"""

from setuptools import setup, find_packages
import os

# Read README for long description
readme_path = os.path.join(os.path.dirname(__file__), 'README.md')
if os.path.exists(readme_path):
    with open(readme_path, 'r', encoding='utf-8') as f:
        long_description = f.read()
else:
    long_description = "GXC Miner Pro - Ultimate Mining Software"

setup(
    name='gxc-miner-pro',
    version='2.0.0',
    author='GXC Foundation',
    author_email='dev@gxc.network',
    description='GXC Miner Pro - Ultimate Mining Software for GXC Blockchain',
    long_description=long_description,
    long_description_content_type='text/markdown',
    url='https://github.com/gxc-network/gxc-miner-pro',
    license='MIT',
    
    packages=find_packages(where='src'),
    package_dir={'': 'src'},
    
    py_modules=['gxc_miner_pro'],
    
    install_requires=[
        'customtkinter>=5.2.0',
        'matplotlib>=3.7.0',
        'psutil>=5.9.0',
        'requests>=2.28.0',
    ],
    
    extras_require={
        'full': [
            'pycryptodome>=3.18.0',
        ],
        'build': [
            'pyinstaller>=6.0.0',
        ],
    },
    
    entry_points={
        'console_scripts': [
            'gxc-miner-pro=gxc_miner_pro:main',
            'gxc-miner=gxc_miner_pro:main',
        ],
        'gui_scripts': [
            'gxc-miner-pro-gui=gxc_miner_pro:main',
        ],
    },
    
    python_requires='>=3.8',
    
    classifiers=[
        'Development Status :: 5 - Production/Stable',
        'Intended Audience :: End Users/Desktop',
        'License :: OSI Approved :: MIT License',
        'Operating System :: OS Independent',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Programming Language :: Python :: 3.10',
        'Programming Language :: Python :: 3.11',
        'Programming Language :: Python :: 3.12',
        'Topic :: Security :: Cryptography',
    ],
    
    keywords='cryptocurrency mining blockchain gxc sha256 ethash gxhash pool solo',
    
    project_urls={
        'Documentation': 'https://docs.gxc.network/miner',
        'Source': 'https://github.com/gxc-network/gxc-miner-pro',
        'Bug Reports': 'https://github.com/gxc-network/gxc-miner-pro/issues',
    },
)
