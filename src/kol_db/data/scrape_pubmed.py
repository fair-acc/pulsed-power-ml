import urllib.error
from datetime import date
from http.client import IncompleteRead
from pathlib import Path
from time import sleep

import pandas as pd
from Bio import Entrez
from tqdm import tqdm


def get_id_list(max_results, term):
    """
    Use Entrez API to get the Ids of search results

    Parameters
    ----------
    max_results : int
        Max number of results to return
    term : str
        Search term

    Returns
    -------
    List[List[int]]
        List of PubMed Ids
    """

    # TODO: EMail as parameter
    Entrez.email = 'stefan.foerstel@infoteam.de'
    retmax = 10000

    retstart = 0
    while retstart < max_results:
        with Entrez.esearch(db='pubmed', retstart=retstart, retmax=retmax, term=term) as handle:
            extracted_ids = Entrez.read(handle)['IdList']
            if len(extracted_ids) == 0:
                break
            else:
                yield extracted_ids

        retstart += retmax


def get_record_info(rec, search_term, rel_person):
    """
    Get information from provided database records

    Parameters
    ----------
    rec : dict
        Provided information from Entrez query
    search_term : str
        Term that was used to search pubmed db
    rel_person : str
        Person who ordered the search

    Returns
    -------
    List[Publication]
        Extracted information
    """

    publications = []
    for article in rec['PubmedArticle']:

        try:
            authors = []
            for aut in article['MedlineCitation']['Article']['AuthorList']:
                if 'CollectiveName' in aut:
                    continue
                authors.append(
                    dict(forename=aut['ForeName'], initials=aut['Initials'], lastname=aut['LastName'],
                         affiliations=[aff['Affiliation'] for aff in aut['AffiliationInfo']])
                )

            try:
                article_date = date(
                    int(article['MedlineCitation']['Article']['ArticleDate'][0]['Year']),
                    int(article['MedlineCitation']['Article']['ArticleDate'][0]['Month']),
                    int(article['MedlineCitation']['Article']['ArticleDate'][0]['Day'])
                )
            except IndexError:
                article_date = None

            doi = None
            try:
                for loc_id in article['MedlineCitation']['Article']['ELocationID']:
                    if getattr(loc_id, 'attributes')['EIdType'] == 'doi':
                        doi = loc_id
                        break
            except IndexError:
                pass

            keywords = []
            for mesh_heading in article['MedlineCitation']['MeshHeadingList']:
                keywords.append(mesh_heading['DescriptorName'])

            pub = dict(
                pubmed_id=article['MedlineCitation']['PMID'],
                doi=doi,
                title=article['MedlineCitation']['Article']['ArticleTitle'],
                authors=authors,
                article_date=article_date,
                journal=article['MedlineCitation']['Article']['Journal']['Title'],
                journal_issn=article['MedlineCitation']['Article']['Journal']['ISSN'],
                journal_iso_abbr=article['MedlineCitation']['Article']['Journal']['ISOAbbreviation'],
                search_term=search_term,
                rel_person=rel_person,
                keywords=keywords
            )

            publications.append(pub)
        except KeyError:
            continue

    return publications


def get_publications(term_info, num_results, out_folder):
    """

    Parameters
    ----------
    term_info : (str, str)
    num_results : int
    out_folder : Path

    Returns
    -------
    List[Publication]

    """

    search_term, rel_person = term_info

    # TODO: Remove IDs already in database
    for sub_list in tqdm(get_id_list(num_results, search_term),
                         desc='Search Results',
                         total=max(1, int(round(num_results / 10000, 0)))):

        if len(sub_list) == 0:
            continue

        out_path = out_folder / sub_list[0]
        if (out_path / 'pubs.csv').is_file() and (out_path / 'authors.csv').is_file():
            continue

        try_counter = 0
        while True:
            if try_counter >= 10:
                break
            with Entrez.efetch(db='pubmed', id=','.join(sub_list), retmode='xml') as handle:
                try:
                    record = Entrez.read(handle, validate=True, escape=False)
                    rec_info = get_record_info(record, search_term, rel_person)
                    pubs_to_csv(rec_info, out_path)
                    break
                except IncompleteRead:
                    sleep(5)
                except (ConnectionResetError, urllib.error.HTTPError):
                    sleep(300)
                finally:
                    try_counter += 1


def pubs_to_csv(data, path):
    """

    Parameters
    ----------
    data : List
    path : Path

    """

    path.mkdir(parents=True, exist_ok=True)

    pubs = pd.DataFrame(data)

    # write pubs
    pubs.drop(columns=['authors', 'journal', 'journal_iso_abbr', 'keywords']) \
        .to_csv(path / 'pubs.csv', index=False, encoding='utf-8-sig', sep=';', decimal=',')

    # write journals
    pubs[['pubmed_id', 'journal', 'journal_issn', 'journal_iso_abbr']] \
        .to_csv(path / 'journals.csv', index=False, encoding='utf-8-sig', sep=';', decimal=',')

    # write keywords
    pubs[['pubmed_id', 'keywords']] \
        .explode('keywords') \
        .to_csv(path / 'keywords.csv', index=False, encoding='utf-8-sig', sep=';', decimal=',')

    # write authors
    authors_helper = pubs[['pubmed_id', 'authors']] \
        .explode('authors')

    authors = pd.concat([
        authors_helper.drop(columns=['authors']),
        authors_helper['authors'].apply(pd.Series)
    ], axis=1) \
        .explode('affiliations') \
        .reset_index(drop=True)

    if 0 in authors.columns:
        authors = authors.drop(columns=[0])

    authors.to_csv(path / 'authors.csv', index=False, encoding='utf-8-sig', sep=';', decimal=',')


def main():
    num_results = 1000000

    search_terms = [
        ('IVUS', 'Ulrike Haberland'),
        ('Cardiology', 'Ulrike Haberland'),
        ('CT (Computed Tomography) guided PCI (Percutaneous Coronary Intervention)', 'Ulrike Haberland'),
        ('OCT', 'Ulrike Haberland'),
        ('Intravascular Imaging', 'Ulrike Haberland'),
        #
        ('Rheumatology', 'Andre Wichmann'),
        ('Rheumatoid Arthritis', 'Andre Wichmann'),
        ('Machine Learning', 'Andre Wichmann'),
        ('Telehealth', 'Andre Wichmann'),
        ('Digital Therapeutics', 'Andre Wichmann'),
        ('Clinical Support', 'Andre Wichmann'),
        ('Drug Tapering', 'Andre Wichmann'),
        ('Therapy Deescalation', 'Andre Wichmann'),
    ]

    for term in search_terms:
        print(f'Starting for search term: {term}')
        sleep(1)
        save_path = Path(f'data/raw/{term[0].replace(" ", "-")}_{num_results}_{date.today()}')

        get_publications(term_info=term, num_results=num_results, out_folder=save_path)


if __name__ == '__main__':
    main()
