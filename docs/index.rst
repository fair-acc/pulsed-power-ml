.. QT Textanalysis Prototype documentation master file, created by
  sphinx-quickstart on Wed Oct 21 13:57:20 2020.
  You can adapt this file completely to your liking, but it should at least
  contain the root `toctree` directive.

QT Textanalysis Prototype
=========================

.. toctree::
   :maxdepth: 2
   :caption: Contents:

The first part of the documentation is based on the README.md in the python project folder. The second part describes
the code itself.

Project Organization
--------------------

.. literalinclude:: ../README.md
   :lines: 10-52

.. include:: ../README.md
   :start-after: include_start
   :end-before: [comment]: before_graph

.. image:: ../references/TaskGraph2.png

.. include:: ../README.md
   :start-after: after_graph

Luigi Documentation
-------------------
For the documentation of the Luigi task structure, see https://luigi.readthedocs.io/en/stable/tasks.html

Code Documentation
==================

General information
-------------------
All steps in the process produce .csv output files that can be persisted as needed.

.. autofunction:: src.data.general_data_helpers.write_to_csv


Raw data processing
-------------------
.. image:: ../references/TaskGraph2_LoadData.png

.. automodule:: src.data.load_ticket_data
   :members:

.. automodule:: src.data.load_blacklist_data
   :members:

Splitting data for anonymization
--------------------------------
.. image:: ../references/TaskGraph2_SplitTicketTexts.png

.. automodule:: src.data.split_ticket_texts
   :members:
   :private-members:

Extraction of ticket texts and features
---------------------------------------
.. image:: ../references/TaskGraph2_ExtractTextBlocks.png

.. automodule:: src.data.extract_texts
   :members:
   :private-members:

Pipeline functions
^^^^^^^^^^^^^^^^^^
.. autofunction:: src.data.get_textblock_info.get_textblock_info
.. autofunction:: src.data.extraction_helpers.extract_text_blocks
.. autofunction:: src.data.extraction_helpers.extract_groups
.. autofunction:: src.data.extraction_helpers.remove_unused_cols
.. autofunction:: src.data.extraction_helpers.merge_list_cols_to_strings
.. autofunction:: src.data.extraction_helpers.extract_error_code
.. autofunction:: src.data.extraction_helpers.extract_error_string
.. autofunction:: src.data.extraction_helpers.extract_features
.. autofunction:: src.data.extraction_helpers.remove_bad_substrings
.. autofunction:: src.data.extraction_helpers.add_full_stop_at_end_of_sentence
.. autofunction:: src.data.extraction_helpers.add_whitespace_around_special_characters
.. autofunction:: src.data.extraction_helpers.process_part_numbers

.. autofunction:: src.data.general_data_helpers.split_extracted_text
.. autofunction:: src.data.general_data_helpers.convert_to_long_format
.. autofunction:: src.data.extraction_helpers.process_report_codes
.. autofunction:: src.data.general_data_helpers.explode_feature_lists
.. autofunction:: src.features.general_feature_helpers.split_error_codes_and_dump_id
.. autofunction:: src.data.general_data_helpers.remove_empty_rows

Spellchecker
^^^^^^^^^^^^
.. automodule:: src.features.spellchecker
   :members:

.. automodule:: src.features.spellchecker_dict_handling
   :members:

.. autofunction:: src.features.spellchecker_dict_generation.generate_replacements

Concatenation of extracted text and features
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. image:: ../references/TaskGraph2_Concat.png

.. automodule:: src.data.concat_extracted_texts
   :members:

.. automodule:: src.data.concat_extracted_features
   :members:

Processing of extracted texts
-----------------------------
.. image:: ../references/TaskGraph2_process_texts.png

.. automodule:: src.features.tokenize_text
   :members:
   :private-members:

.. automodule:: src.features.filter_tokens
   :members:

.. automodule:: src.features.remove_blacklist_tokens
   :members:
   :private-members:

Processing of extracted features
--------------------------------
.. image:: ../references/TaskGraph2_process_features.png

.. automodule:: src.features.process_features
   :members:
   :private-members:

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
