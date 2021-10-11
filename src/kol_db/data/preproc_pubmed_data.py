from pathlib import Path

import pandas as pd
from tqdm import tqdm


def build_aff_id(df):
    """

    Parameters
    ----------
    df : pd.DataFrame

    Returns
    -------

    """

    df_unique = df[['affiliations']] \
        .copy() \
        .drop_duplicates() \
        .reset_index(drop=True)
    df_unique['affiliation_id'] = df_unique.index

    return df.merge(df_unique, how='left', on='affiliations')


def build_author_id(df):
    """

    Parameters
    ----------
    df : pd.DataFrame

    Returns
    -------

    """

    df_unique = df[['forename', 'initials', 'lastname']] \
        .copy() \
        .drop_duplicates() \
        .reset_index(drop=True)
    df_unique['author_id'] = df_unique.index

    return df.merge(df_unique, how='left', on=['forename', 'initials', 'lastname'])


def build_keyword_id(df):
    """

    Parameters
    ----------
    df : pd.DataFrame

    Returns
    -------

    """

    df_unique = df[['keywords']] \
        .copy() \
        .drop_duplicates() \
        .reset_index(drop=True)
    df_unique['keyword_id'] = df_unique.index

    return df.merge(df_unique, how='left', on='keywords')


def merge_csv_files():
    """"""

    rows_to_consider = 100

    raw_folder_path = Path(f'data/raw')
    proc_folder_path = Path(f'data/processed')

    for file_type in tqdm(['authors', 'journals', 'keywords', 'pubs']):
        data = pd.concat([pd.read_csv(f, encoding='utf-8-sig', sep=';', decimal=',', on_bad_lines='warn',
                                      dtype=str, nrows=rows_to_consider)
                          for f in raw_folder_path.rglob(f'{file_type}.csv')], axis=0)

        if file_type == 'authors':
            data = data \
                .pipe(build_aff_id) \
                .pipe(build_author_id)

        if file_type == 'keywords':
            data = build_keyword_id(data)

        data.to_csv(proc_folder_path / f'{file_type}.csv', index=False, encoding='utf-8-sig', sep=';', decimal=',')


def create_data_improv_files():
    """"""

    authors_path = Path(f'data/processed/authors.csv')
    authors = pd.read_csv(authors_path,
                          encoding='utf-8-sig', sep=';', decimal=',', on_bad_lines='warn')

    authors[['affiliations']] \
        .drop_duplicates() \
        .sort_values(by=['affiliations']) \
        .to_csv(Path(f'data/processed/proof/affiliations.csv'),
                index=False, encoding='utf-8-sig', sep=';', decimal=',')

    authors[['forename', 'initials', 'lastname', 'affiliations']] \
        .drop_duplicates() \
        .sort_values(by=['lastname', 'forename', 'initials']) \
        .to_csv(Path(f'data/processed/proof/authors.csv'),
                index=False, encoding='utf-8-sig', sep=';', decimal=',')


if __name__ == '__main__':
    merge_csv_files()
    # create_data_improv_files()
    # TODO: Build Website for Data Cleaning
