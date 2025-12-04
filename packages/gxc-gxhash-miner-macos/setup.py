#!/usr/bin/env python3
from setuptools import setup
setup(
    name='gxc-gxhash-miner',
    version='2.0.0',
    py_modules=['gxhash_miner'],
    install_requires=['customtkinter>=5.2.0', 'psutil>=5.9.0', 'requests>=2.28.0'],
    entry_points={'console_scripts': ['gxhash-miner=gxhash_miner:main']},
    python_requires='>=3.8',
)
