from dataclasses import dataclass
from typing import List


@dataclass()
class Author:
    """Class storing information about scraped author."""
    affiliations: List[str]
    forename: str = None
    initials: str = None
    lastname: str = None
