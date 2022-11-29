from sklearn.decomposition import PCA
from sklearn.pipeline import make_pipeline
from sklearn.preprocessing import StandardScaler, MinMaxScaler, MaxAbsScaler, RobustScaler
from sklearn.metrics import accuracy_score
from sklearn.metrics import confusion_matrix 
from sklearn.metrics import ConfusionMatrixDisplay

import matplotlib.pyplot as plt
import numpy as np

from src.pulsed_power_ml.models.gupta_model.gupta_clf import GuptaClassifier
from src.pulsed_power_ml.models.gupta_model.gupta_utils import read_power_data_base
from src.pulsed_power_ml.models.gupta_model.gupta_utils import read_parameters


def evaluate_knn_classifier(parameter_file: str, 
                            power_db_file: str,
                            training_folder: str,
                            validation_folder: str,
                            n_neighb: int,
                            knn_weights,
                            n_known_appliances: int = 11,
                            distance_thres: float = np.inf, 
                            scaler: str = "NoScaler") -> float:
    """
    Evaluation method for the knn classifier.

    Parameters
    ----------
    parameter_file:
        Path to parameter file.
    power_db_file:
        Path to power database file.
    training_folder:
        Path to training data, containing Features_ApparentPower_0.7_p.csv 
        and Labels_ApparentPower_0.7_p.csv
    validation_folder:
        Path to validation data, containing Features_ApparentPower_0.3_p.csv 
        and Labels_ApparentPower_0.3_p.csv
    n_known_appliances
        Number of known appliances.
    n_neighbors
        Number of nearest neighbors which should be checked during the classification.
    knn_weights
        How to weight the distance of al neighbors.
    distance_threshold
        If distance to the nearest neighbor is above this threshold, an event is classified as "other"
    """
   
    apparent_power_list = read_power_data_base(power_db_file)
    parameter_dict = read_parameters(parameter_file)

    clf = GuptaClassifier(
        background_n=parameter_dict["background_n"],
        fft_size_real=parameter_dict["fft_size"] / 2,
        sample_rate=parameter_dict["sample_rate"],
        n_known_appliances=11, # just for this test
        spectrum_type=2,
        n_peaks_max=10,
        apparent_power_list=apparent_power_list,
        n_neighbors=n_neighb,
        distance_threshold=distance_thres
    )

    # Load training data
    X = np.genfromtxt(training_folder + "Features_ApparentPower_0.7_p.csv",
                    delimiter=",")
    y = np.genfromtxt(training_folder + "Labels_ApparentPower_0.7_p.csv",
                    delimiter=",")

    yy = y.argmax(axis=1)


    # Load validation data
    X_val = np.genfromtxt(validation_folder + "Features_ApparentPower_0.3_p.csv", 
                        delimiter=",")
    y_val = np.genfromtxt(validation_folder + "Labels_ApparentPower_0.3_p.csv",
                        delimiter=",")

    yy_val = y_val.argmax(axis=1)

    if scaler == "NoScaler":
        # raw features, without scaling
        knn = clf.clf

        knn.fit(X, yy)

        prediction = knn.predict(X_val)
        score = accuracy_score(prediction, yy_val)
        print(score)

        ConfusionMatrixDisplay.from_estimator(knn, X_val, yy_val)
        plt.title("{0}, n_neighbors: {1} , accuracy score: {2}".format(scaler,
                                                            clf.n_neighbors,
                                                            score))
        plt.show()

    else:
        
        if scaler == "StandardScaler":
            sc = StandardScaler()
        elif scaler == "MinMaxScaler":
            sc = MinMaxScaler()
        elif scaler == "MaxAbsScaler":
            sc = MaxAbsScaler()
        elif scaler == "RobustScaler":
            sc = RobustScaler()
        else:
            raise ValueError("Scaler must be one of [NoScaler, StandardScaler, MinMaxScaler, MaxAbsScaler, RobustScaler]!")

        
        # Try again with normalized data
        knn_norm = clf.clf

        X_train = sc.fit_transform(X)
        X_test = sc.transform(X_val) 

        knn_norm.fit(X_train, yy)

        prediction_norm = knn_norm.predict(X_test)
        score = accuracy_score(prediction_norm, yy_val)
        print(score)

        # cm = confusion_matrix(yy_val, prediction_norm)
        # print(cm)

        ConfusionMatrixDisplay.from_estimator(knn_norm, X_test, yy_val)
        plt.title("{0}, n_neighbors: {1} , accuracy score: {2}".format(scaler,
                                                                    clf.n_neighbors,
                                                                    score))
        plt.show()

    return score


if __name__ == "__main__":
    power_db_file = "src/pulsed_power_ml/models/gupta_model/apparent_power_data_base.yml"
    parameter_file = "src/pulsed_power_ml/models/gupta_model/parameters.yml"
    training_data_folder = "../training_data/labels_20221123/"
    validation_data_folder = '../training_data/labels_20221123_validation/'

    score = evaluate_knn_classifier(parameter_file, 
                                    power_db_file,
                                    training_data_folder,
                                    validation_data_folder,
                                    n_neighb=7,
                                    knn_weights=None,
                                    scaler="NoScaler")