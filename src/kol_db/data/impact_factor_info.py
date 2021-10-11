from pathlib import Path

import nltk
import pandas as pd
from iso4 import abbreviate

nltk.download('wordnet')


def abbr_title(title):
    """"""

    try:
        return abbreviate(title, False, disambiguation_langs=['eng'])
    except Exception:
        print(title)
        return ''


if_path = Path('data/external/impact_factors.csv')
out_path = Path('data/external/impact_factors_abbr.csv')

if_data = pd.read_csv(if_path, sep=';', decimal=',', thousands='.')
if_data['abbrev'] = if_data['Full Journal Title'].apply(abbr_title)

if_data.to_csv(out_path, sep=';', decimal=',', index=False, encoding='utf-8-sig')
