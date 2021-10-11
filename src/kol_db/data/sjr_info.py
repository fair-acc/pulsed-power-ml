from pathlib import Path

import pandas as pd

sjr_path = Path('data/external/scimagojr 2020.csv')
out_path = Path('data/external/sjr_2020.csv')

sjr_data = pd.read_csv(sjr_path, sep=';', decimal=',')
sjr_data['Issn'] = sjr_data['Issn'].apply(lambda x: x.split(','))

sjr_data = sjr_data.explode('Issn')
sjr_data.to_csv(out_path, index=False, sep=';', decimal=',')

#

journal_data_path = Path('data/processed/journals.csv')
journal_data = pd.read_csv(journal_data_path, sep=';', decimal=',')

journal_data['journal_issn'] = journal_data['journal_issn'].apply(lambda x: x.replace('-', ''))

merged_journal_data = journal_data.merge(sjr_data, left_on='journal_issn', right_on='Issn')
merged_journal_data.to_csv('data/processed/joined_journals.csv', index=False, sep=';', decimal=',')
