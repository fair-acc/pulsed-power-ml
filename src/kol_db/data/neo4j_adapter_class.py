from typing import Dict, TypeVar, Generic

import pandas as pd
from neo4j import GraphDatabase

T = TypeVar('T')


class Neo4jError(Exception):
    """Exception for Neo4jConnector"""
    pass


class Neo4jConnector(Generic[T]):
    """Adapter class for Neo4j"""

    def __init__(self, uri, user, password):
        self.driver = GraphDatabase.driver(uri, auth=(user, password))

    def __enter__(self):
        """Defines what happens upon entering context manager"""

        return self.driver.session()

    def __exit__(self, exc_type, exc_val, exc_tb):
        """Defines what happens upon exiting context manager"""

        self.driver.close()

    def __repr__(self):
        """Define a representation for class if called"""

        return "%s(%r)" % (self.__class__.__name__, self.__dict__)


if __name__ == '__main__':
    user = 'neo4j'
    password = 'n5qBoVe4FCSkECSeMpm5'

    def _create_and_return_greeting(tx, message):
        result = tx.run("CREATE (a:Greeting) "
                        "SET a.message = $message "
                        "RETURN a.message + ', from node ' + id(a)", message=message)
        return result.single()[0]

    with Neo4jConnector('bolt://localhost:7687', user=user, password=password) as session:
        greeting = session.write_transaction(_create_and_return_greeting, "hello, world")
        print(greeting)
