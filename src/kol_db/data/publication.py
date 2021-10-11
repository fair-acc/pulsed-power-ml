from dataclasses import dataclass
from datetime import date
from pathlib import Path
from typing import List

from src.kol_db.data.author import Author
from src.kol_db.data.pickle_helper import load_pickled_object, save_object


@dataclass()
class Publication:
    """Class storing information about scraped publication."""
    pubmed_id: str
    doi: str

    title: str
    authors: List[Author]
    article_date: date
    journal: str
    journal_iso_abbr: str

    search_term: str
    keyword_list: List[str]


if __name__ == '__main__':
    pub = [
        Publication('Test', 'DOI', 'Title', [Author(['aff1', ]), ],
                    date(2021, 1, 1), 'Journal', 'Journal Abbr', 'Term', ['A', ]),
        Publication('Test', 'DOI', 'Title', [Author(['aff1', ]), ],
                    date(2021, 1, 1), 'Journal', 'Journal Abbr', 'Term', ['A', ]),
    ]

    out_path = Path('data/raw/tmp.pickle')
    save_object(pub, out_path)
    dat = load_pickled_object(out_path)

    print(dat)
