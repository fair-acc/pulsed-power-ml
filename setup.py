from setuptools import find_packages, setup

# Todo: Add license and description texts
setup(
    name='kol_db',
    #packages=find_packages(exclude=("test",)),  # Exclude tests for enduser
    packages=find_packages("src"),  # include all packages under src
    package_dir={"": "src"},  # tell distutils packages are under src
    version='0.1.0',
    description='TODO',
    author='infoteam Software',
    license='',
    install_requires=['luigi==3.0.2',   # Packages needed by enduser, not developer
                      'pandas==1.1.5',
                      ],
    entry_points={'console_scripts': ['kol_db=src.kol_db.main:main'],
                  },
)
