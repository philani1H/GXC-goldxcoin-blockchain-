#!/usr/bin/env python3
"""
Setup script for GXC GXHash Miner - Python Package
Makes the miner installable via pip: pip install gxc-gxhash-miner
"""

from setuptools import setup, find_packages
import os

# Read the README file if it exists
readme_file = os.path.join(os.path.dirname(__file__), "README.md")
long_description = ""
if os.path.exists(readme_file):
    with open(readme_file, "r", encoding="utf-8") as f:
        long_description = f.read()

# Read requirements
requirements_file = os.path.join(os.path.dirname(__file__), "gxhash_miner_requirements.txt")
requirements = []
if os.path.exists(requirements_file):
    with open(requirements_file, "r", encoding="utf-8") as f:
        requirements = [line.strip() for line in f if line.strip() and not line.startswith("#")]

setup(
    name="gxc-gxhash-miner",
    version="1.0.0",
    description="GXC GXHash Miner - Python implementation with GUI for mining GXC blockchain",
    long_description=long_description,
    long_description_content_type="text/markdown",
    author="GXC Development Team",
    author_email="support@gxc.network",
    url="https://github.com/gxc-blockchain/miners",
    py_modules=["gxhash_miner"],
    install_requires=requirements,
    python_requires=">=3.7",
    entry_points={
        "console_scripts": [
            "gxc-gxhash-miner=gxhash_miner:main",
        ],
    },
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: End Users/Desktop",
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
        "Operating System :: OS Independent",
        "Topic :: Office/Business :: Financial",
        "Topic :: Internet",
    ],
    keywords="blockchain cryptocurrency mining gxc gxhash",
    project_urls={
        "Documentation": "https://docs.gxc.network",
        "Source": "https://github.com/gxc-blockchain/miners",
        "Tracker": "https://github.com/gxc-blockchain/miners/issues",
    },
)
