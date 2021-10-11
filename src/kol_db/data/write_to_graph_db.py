from itertools import combinations
from pathlib import Path

import pandas as pd
from py2neo import Graph
from py2neo.bulk import merge_nodes, merge_relationships
from tqdm import tqdm


def chunks(lst, n):
    """Yield successive n-sized chunks from lst."""

    for i in range(0, len(lst), n):
        yield lst[i:i + n]


def create_pub_nodes(graph, pub_data):
    """
    Create neo4j nodes for all publications

    Parameters
    ----------
    graph : Graph
        py2neo connection to db
    pub_data : pd.DataFrame
        Data for all publications
    """

    keys = ['idPubmed', 'title', 'articleDate', 'doi', 'searchTerm']
    data = [[row['pubmed_id'], row['title'], row['article_date'], row['doi'], row['search_term']]
            for _, row in pub_data.iterrows()]

    for sub_data in tqdm(chunks(data, 5000), desc='Create Pub nodes'):
        merge_nodes(graph.auto(), sub_data, merge_key=('Publication', 'idPubmed'), keys=keys)


def create_journal_nodes(graph, journal_data):
    """
    Create neo4j nodes for all journals

    Parameters
    ----------
    graph : Graph
        py2neo connection to db
    journal_data : pd.DataFrame
        Data for all publications
    """

    df = journal_data[['journal_issn', 'Title', 'SJR']] \
        .drop_duplicates()

    keys = ['issn', 'title', 'sjr']
    data = [[row['journal_issn'], row['Title'], row['SJR']] for _, row in df.iterrows()]

    for sub_data in tqdm(chunks(data, 5000), desc='Create Journal nodes'):
        merge_nodes(graph.auto(), sub_data, merge_key=('Journal', 'issn'), keys=keys)


def create_pub_journal_relationships(graph, journal_data):
    """
    Create neo4j relationships for author_data and publications

    Parameters
    ----------
    graph : Graph
        py2neo connection to db
    journal_data : pd.DataFrame
        Data for all publications
    """

    df = journal_data[['pubmed_id', 'journal_issn']] \
        .drop_duplicates()

    data = [(row['pubmed_id'], {}, row['journal_issn']) for _, row in df.iterrows()]

    for sub_data in tqdm(chunks(data, 5000), desc='Create Journal/Pub relationships'):
        merge_relationships(graph.auto(), sub_data, 'PUBLISHED_BY_JOURNAL',
                            start_node_key=('Publication', 'idPubmed'),
                            end_node_key=('Journal', 'issn'))


def create_keyword_nodes(graph, keyword_data):
    """
    Create neo4j nodes for all journals

    Parameters
    ----------
    graph : Graph
        py2neo connection to db
    keyword_data : pd.DataFrame
        Data for all publications
    """

    df = keyword_data[['keyword_id', 'keywords']] \
        .drop_duplicates()

    keys = ['idKeyword', 'keyword']
    data = [[row['keyword_id'], row['keywords']] for _, row in df.iterrows()]

    for sub_data in tqdm(chunks(data, 5000), desc='Create Keyword nodes'):
        merge_nodes(graph.auto(), sub_data, merge_key=('Keyword', 'idKeyword'), keys=keys)


def create_pub_keyword_relationships(graph, keyword_data):
    """
    Create neo4j relationships for author_data and publications

    Parameters
    ----------
    graph : Graph
        py2neo connection to db
    keyword_data : pd.DataFrame
        Data for all publications
    """

    df = keyword_data[['pubmed_id', 'keyword_id']] \
        .drop_duplicates()

    data = [(int(row['pubmed_id']), {}, int(row['keyword_id'])) for _, row in df.iterrows()]

    for sub_data in tqdm(chunks(data, 5000), desc='Create Keyword/Pub relationships'):
        merge_relationships(graph.auto(), sub_data, 'HAS_KEYWORD',
                            start_node_key=('Publication', 'idPubmed'),
                            end_node_key=('Keyword', 'idKeyword'))


def create_author_nodes(graph, author_data):
    """
    Create neo4j nodes for all Authors

    Parameters
    ----------
    graph : Graph
        py2neo connection to db
    author_data : pd.DataFrame
        Data for all authors
    """

    df = author_data \
        .drop(columns=['affiliations', 'affiliation_id']) \
        .drop_duplicates()

    keys = ['idAuthor', 'title', 'forename', 'initials', 'lastname']
    data = [[row['author_id'], f'{row["lastname"]}, {row["forename"]}', row["forename"], row["initials"],
             row["lastname"]] for _, row in df.iterrows()]

    for sub_data in tqdm(chunks(data, 5000), desc='Create Author nodes'):
        merge_nodes(graph.auto(), sub_data,
                    merge_key=('Author', 'idAuthor'),
                    keys=keys)


def create_affiliation_nodes(graph, author_data):
    """
    Create neo4j nodes for all Affiliations

    Parameters
    ----------
    graph : Graph
        py2neo connection to db
    author_data : pd.DataFrame
        Data for all authors
    """

    df = author_data[['affiliations', 'affiliation_id']] \
        .drop_duplicates()

    keys = ['idAffiliation', 'affiliation']
    data = [[row['affiliation_id'], row['affiliations']] for _, row in df.iterrows()]

    for sub_data in tqdm(chunks(data, 5000), desc='Create Affiliation nodes'):
        merge_nodes(graph.auto(), sub_data,
                    merge_key=('Affiliation', 'idAffiliation'),
                    keys=keys)


def create_author_pub_relationships(graph, author_data):
    """
    Create neo4j relationships for author_data and publications

    Parameters
    ----------
    graph : Graph
        py2neo connection to db
    author_data : pd.DataFrame
        Data for all authors
    """

    df = author_data \
        .drop(columns=['affiliations']) \
        .drop_duplicates()

    data = [((row['author_id']), {}, row['pubmed_id'])
            for _, row in df.iterrows()]

    for sub_data in tqdm(chunks(data, 5000), desc='Create Author/Publication relationships'):
        merge_relationships(graph.auto(), sub_data, 'AUTHORED_PUBLICATION',
                            start_node_key=('Author', 'idAuthor'),
                            end_node_key=('Publication', 'idPubmed'))


def create_author_aff_relationships(graph, author_data):
    """
    Create neo4j relationships for author_data and affiliations

    Parameters
    ----------
    graph : Graph
        py2neo connection to db
    author_data : pd.DataFrame
        Data for all authors
    """

    data = [((row['author_id']), {}, row['affiliation_id'])
            for _, row in author_data.iterrows()]

    for sub_data in tqdm(chunks(data, 5000), desc='Create Author/Affiliation relationships'):
        merge_relationships(graph.auto(), sub_data, 'HAS_AFFILIATION',
                            start_node_key=('Author', 'idAuthor'),
                            end_node_key=('Affiliation', 'idAffiliation'))


def update_author_author_relationships(graph, author_data):
    """
    Create neo4j relationships authors

    Parameters
    ----------
    graph : Graph
        py2neo connection to db
    author_data : pd.DataFrame
        Data for all authors
    """

    author_id_pairs = author_data[['author_id', 'pubmed_id']] \
        .drop_duplicates() \
        .groupby('pubmed_id')['author_id'] \
        .apply(list) \
        .apply(lambda x: list(combinations(x, 2)) if len(x) > 1 else None) \
        .dropna() \
        .explode() \
        .values

    for sub_data in tqdm(chunks(author_id_pairs, 5000), desc='Update Author/Author relationships'):
        params = [{'from': a, 'to': b} for a, b in sub_data]
        query = f"""
                UNWIND $params AS row
                MATCH (a:Author)
                WHERE a.idAuthor = row.from
                MATCH (b:Author)
                WHERE b.idAuthor = row.to
                WITH a, b
                MERGE (a)-[r:IS_CONNECTED]-(b)
                    ON CREATE
                        SET r.common_pubs = 1
                    ON MATCH
                        SET r.common_pubs = r.common_pubs + 1
                """

        graph.run(query, parameters={'params': params})


def create_data_dict(file_types, base_path, rows_per_term):
    """

    Parameters
    ----------
    file_types : List[str]
    base_path : Path
    rows_per_term : int

    Returns
    -------
    dict

    """

    data_dict = {}
    if 'pubs' in file_types:
        pub_data_full = pd.read_csv(base_path / 'pubs.csv', encoding='utf-8-sig', sep=';', decimal=',')
        pub_data_full = pub_data_full[pub_data_full.rel_person == 'Andre Wichmann']

        pub_ids = pub_data_full.groupby('search_term')['pubmed_id'].head(rows_per_term).unique()
        data_dict['pubs'] = pub_data_full[pub_data_full['pubmed_id'].isin(pub_ids)]
    else:
        pub_ids = [1]

    for ft in tqdm(file_types, desc='Load data'):
        if ft == 'pubs':
            continue

        data = pd.read_csv(base_path / f'{ft}.csv', encoding='utf-8-sig', sep=';', decimal=',')

        if 1 not in pub_ids:
            data_dict[ft] = data[data['pubmed_id'].isin(pub_ids)]
        else:
            data_dict[ft] = data

    return data_dict


if __name__ == '__main__':
    num_rows_to_load = 10000
    processed_files_path = Path('D:/Dropbox/infoteam/Projekte/P01837_KOL_DB/data/processed')
    file_types_to_process = ['authors', 'journals', 'keywords', 'pubs']

    data_dict = create_data_dict(file_types_to_process, processed_files_path, num_rows_to_load)

    graph = Graph('bolt://localhost:7687', auth=('neo4j', 'n5qBoVe4FCSkECSeMpm5'))
    graph.run('CREATE INDEX IF NOT EXISTS FOR (p:Publication) ON (p.idPubmed)')
    graph.run('CREATE INDEX IF NOT EXISTS FOR (j:Journal) ON (j.issn)')
    graph.run('CREATE INDEX IF NOT EXISTS FOR (k:Keyword) ON (k.idKeyword)')
    graph.run('CREATE INDEX IF NOT EXISTS FOR (a:Author) ON (a.idAuthor)')
    graph.run('CREATE INDEX IF NOT EXISTS FOR (aff:Affiliation) ON (aff.idAffiliation)')

    # pubs
    create_pub_nodes(graph, data_dict['pubs'])

    # journals
    create_journal_nodes(graph, data_dict['journals'])
    create_pub_journal_relationships(graph, data_dict['journals'])

    # keywords
    create_keyword_nodes(graph, data_dict['keywords'])
    create_pub_keyword_relationships(graph, data_dict['keywords'])

    # authors
    create_author_nodes(graph, data_dict['authors'])
    create_affiliation_nodes(graph, data_dict['authors'])
    create_author_pub_relationships(graph, data_dict['authors'])
    create_author_aff_relationships(graph, data_dict['authors'])

    # Common publication counts
    update_author_author_relationships(graph, data_dict['authors'])
