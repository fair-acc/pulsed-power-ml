import dill


def save_object(obj, path):
    """Save scraped data to disk"""

    try:
        with open(path, 'wb') as f:
            dill.dump(obj, f)
    except Exception as ex:
        print('Error during pickling object (Possibly unsupported):', ex)


def load_pickled_object(filename):
    """"""

    try:
        with open(filename, "rb") as f:
            return dill.load(f)
    except Exception as ex:
        print("Error during unpickling object (Possibly unsupported):", ex)
