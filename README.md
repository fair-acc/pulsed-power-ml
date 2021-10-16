Project GSI/FAIR - Energiemanagement
==============================

> We don't just trust people to obey the rules, we also trust that they know when to break them. - *Simon Sinek*

Project Organization
--------------------

        +-- LICENSE
        +-- README.md                 <- The top-level README for developers using this project.
        +-- data        
        |   +-- external              <- Data from third party sources.
        |   +-- interim               <- Intermediate data that has been transformed.
        |   +-- processed             <- The final, canonical data sets for modeling.
        |   +-- raw                   <- The original, immutable data dump.
        |       
        +-- docs                      <- A default Sphinx project; see sphinx-doc.org for details
        |       
        +-- logs                      <- A folder to save log files to
        |       
        +-- models                    <- Trained and serialized models, or model summaries
        |       
        +-- notebooks                 <- Jupyter notebooks. Naming convention is a number (for ordering),
        |                                the creator's initials, and a short `-` delimited description, e.g.
        |                                 `1.0-jqp-initial-data-exploration`.
        |       
        +-- references                <- Data dictionaries, manuals, and all other explanatory materials.
        |       
        +-- reports                   <- Generated analysis as HTML, PDF, LaTeX, etc.
        |   +-- figures               <- Generated graphics and figures to be used in reporting
        |       
        +-- requirements.txt          <- The requirements file for reproducing the analysis environment, e.g.
        |                                generated with `pip freeze > requirements.txt`
        |       
        +-- setup.py                  <- makes project pip installable (pip install -e .)
        +-- src                       <- Source code for use in this project
        |   +-- pulsed_power_ml       <- Rename this folder to project name
        |       +-- __init__.py       <- Makes src a Python module
        |       |
        |       +-- data              <- Scripts to download or generate data
        |       |
        |       +-- features          <- Scripts to turn raw data into features for modeling
        |       |
        |       +-- models            <- Scripts to train models and then use trained models to make predictions
        |       |
        |       +-- visualization     <- Scripts to create exploratory and results oriented visualizations
        +-- test                      <- Tests for source code in this project. Mirrored folder structure to src.

[comment]: include_start 
Raw Data
--------

* Preparation before kick-off meeting:
  1. Create new environment using the [requirements.txt](requirements.txt). More info in the [pip documentation](https://pip.pypa.io/en/stable/user_guide/#requirements-files).
  1. Read the basic info in this template

* After kick-off meeting:
  1. Fill out [project charter](references/project/charter.md).
  1. Add raw data source information to [Data and Feature Definitions](references/data_report/data_definition.md).
  1. Fill out the [Data Report](references/data_report).
  1. Create a data dictionary for each raw dataset.


Basic Info
-----------

The working directory for this project should be the base project folder. Please don't add any code that
manually changes that. If you are working in PyCharm, go to Run -> Edit Configurations and under Templates -> Python 
set your Working Directory to that path. This way, all newly created *.py Files will have it set as default.

Work according to the [GitHub Flow](https://guides.github.com/introduction/flow/).

Before you commit your code, please use the function provided under Code -> Reformat File or reformat your code and
clean up the imports otherwise.
For committing code to the repository please follow this [guide on commit messages](https://datasciencecampus.github.io/coding-standards/version-control.html).

If you add code, please make sure to document it concisely, following this [guide](https://numpydoc.readthedocs.io/en/latest/format.html).
In PyCharm you can change the default docstring style under File -> Settings -> Tools -> Python Integrated Tools -> Docstring format -> NumPy.

Have a look at the [DVC tutorial](https://www.youtube.com/watch?v=kLKBcPonMYw&list=PL7WG7YrwYcnDb0qdPl9-KEStsL-3oaEjg)
or the complete [MLOps tutorial](https://www.youtube.com/watch?v=9BgIDqAzfuA&list=PL7WG7YrwYcnDBDuCkFbcyjnZQrdskFsBz).

Code Structure
--------------

The code structure for this project should be quite simple, as it is a prototype.
Hence, there should be one main.py file (located at src/main.py) which describes the pipeline we build.

Luigi HowTo
-----------

Example Task Setup (please update when changed):

[comment]: before_graph 
TODO: Add TaskGraph for new project
![Luigi Task Graph](references/TaskGraph.png)

[comment]: after_graph 

How to run with central scheduler:

1. Start Luigi Central Scheduler from commandline with command: luigid
2. Open [http://localhost:8082](http://localhost:8082) in your browser.
3. Run Wrapper Module in another commandline with command:
    python -m luigi -\-module main ExampleWrapperTask -\-workers=4
4. Refresh Browser Tab to view running processes
