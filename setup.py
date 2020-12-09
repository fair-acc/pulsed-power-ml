from setuptools import find_packages, setup

# Todo: Add license and description texts
setup(
    name='it_ticket_analysis',
    packages=find_packages(exclude=("test",)),  # Exclude tests for enduser
    version='0.1.0',
    description='TODO',
    author='infoteam Software',
    license='',
    install_requires=['luigi==3.0.2',   # Packages needed by enduser, not developer
                      'pandas==1.1.5',
                      ],
    entry_points={'console_scripts': ['it_ticket_analysis=it_ticket_analysis.main:main'],
                  },
)
