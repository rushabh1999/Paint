from setuptools import setup, find_packages

setup(
    name="drift-detector-cli",
    version="1.0.0",
    description="Configuration Drift Detector with Hash-based Integrity Verification",
    author="Configuration Drift Detector Team",
    python_requires=">=3.7",
    package_dir={"": "src"},
    py_modules=["drift_cli"],
    entry_points={
        "console_scripts": [
            "drift-cli=drift_cli:main",
        ],
    },
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: System Administrators",
        "Topic :: System :: Monitoring",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
    ],
)
