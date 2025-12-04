#!/usr/bin/env python3
from setuptools import setup, find_packages

with open('README.md', 'r', encoding='utf-8') as f:
    long_description = f.read()

setup(
    name='gxc-miner-suite',
    version='2.0.0',
    author='GXC Blockchain',
    author_email='dev@gxc.network',
    description='GXC Miner Suite - SHA256, Ethash, GxHash Mining',
    long_description=long_description,
    long_description_content_type='text/markdown',
    url='https://github.com/gxc-blockchain/gxc-miner-suite',
    packages=find_packages(),
    package_dir={'': '.'},
    install_requires=[
        'customtkinter>=5.2.0',
        'psutil>=5.9.0',
        'requests>=2.28.0',
        'matplotlib>=3.7.0',
    ],
    entry_points={
        'console_scripts': [
            'gxc-miner=src.gxc_miner:main',
            'gxc-sha256=src.sha256_miner:main',
            'gxc-ethash=src.ethash_miner:main',
        ],
        'gui_scripts': [
            'gxc-miner-gui=src.gxc_miner:main',
        ],
    },
    python_requires='>=3.8',
    classifiers=[
        'Development Status :: 4 - Beta',
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Programming Language :: Python :: 3.10',
        'Programming Language :: Python :: 3.11',
        'Operating System :: OS Independent',
        'Topic :: Security :: Cryptography',
        'Intended Audience :: End Users/Desktop',
    ],
    keywords='gxc blockchain mining cryptocurrency sha256 ethash gxhash',
)
