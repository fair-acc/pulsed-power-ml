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


def redefine_labels_different(labels: np.ndarray) -> np.ndarray:
    """
    Method to redefine labels such that there are only different appliances. 
    Events of class L1 and L2 are merged, as well as H1 and H2, R1 and R2, F1 and F2.

    Parameters
    ----------
    labels:
        Array of arrayz as read in from csv files
        
    Returns
    -------
    labels:
        redefined labels.
    """
    merge_cats = [(1,2),
                  (4,5),
                  (6,7),
                  (9,10),
                  (12,13),
                  (15,16),
                  (17,18),
                  (20,21)]
    new_cats = [0,1,2,3,4,6,8,9,11,12,14,15,17,19,20]
    for merge in merge_cats:
        for ii in range(0,labels.__len__()):
            cat = labels[ii].argmax()
            if cat == merge[1]:
                labels[ii] = labels[np.where((labels.argmax(axis=1))==merge[0])[0][0]]
        
        print(labels[:,merge[1]].sum())
    labels = labels[:,new_cats]
    
    return labels


def evaluate_knn_classifier(parameter_file: str, 
                            power_db_file: str,
                            training_folder: str,
                            validation_folder: str,
                            n_neighb: int,
                            knn_weights: str = "distance",
                            n_known_appliances: int = 11,
                            distance_thres: float = np.inf, 
                            scaler: str = "NoScaler",
                            makeplot: bool=True,
                            types: str="all") -> float:
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
    makeplot
        Boolean, if True plot confusion matrix
    types
        Which types of appliances to use for evaluation (training always with all):
        Chose one of {"all", "different", "similar"}
    """
   
    apparent_power_list = read_power_data_base(power_db_file)
    parameter_dict = read_parameters(parameter_file)

    clf = GuptaClassifier(
        background_n=parameter_dict["background_n"],
        fft_size_real=parameter_dict["fft_size"] / 2,
        sample_rate=parameter_dict["sample_rate"],
        n_known_appliances=11, # just for this test
        spectrum_type=2,
        n_peaks_max=parameter_dict["n_peaks"],
        apparent_power_list=apparent_power_list,
        n_neighbors=n_neighb,
        distance_threshold=distance_thres,
        knn_weights=knn_weights,
    )

    # Load training data
    X = np.genfromtxt(training_folder + "Features_ApparentPower_0.7_p.csv",
                    delimiter=",")
    y = np.genfromtxt(training_folder + "Labels_ApparentPower_0.7_p.csv",
                    delimiter=",")

    # Load validation data
    X_val = np.genfromtxt(validation_folder + "Features_ApparentPower_0.3_p.csv", 
                        delimiter=",")
    y_val = np.genfromtxt(validation_folder + "Labels_ApparentPower_0.3_p.csv",
                        delimiter=",")

    knn = clf.clf
    
    if scaler == "NoScaler":
        # raw features, without scaling
        X_train = X
        X_test = X_val 

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
        X_train = sc.fit_transform(X)
        X_test = sc.transform(X_val) 

    # train classifier
    knn.fit(X_train, y)

    prediction = knn.predict(X_test)
    #print(score)
# ToDo: Move this to before fitting the classifier, use redefine_labels function for "different"
    # cm = confusion_matrix(yy_val, prediction)
    # print(cm)
    if types == "different":
        # only evaluate classification of different types of appliances
        pass
    elif types == "similar":
        # only evaluate classification of same types of appliances
        pass
    elif types != "all":
        raise ValueError("types must be one of [different, similar, all]!")
    
    else:
        pass
    
    score = accuracy_score(prediction, y_val)

    if makeplot:
        ConfusionMatrixDisplay.from_estimator(knn, X_test, y_val)
        plt.title("{0}, n_neighbors: {1} , accuracy score: {2}".format(scaler,
                                                                    clf.n_neighbors,
                                                                    score))
        plt.show()

    return score


if __name__ == "__main__":
    power_db_file = "src/pulsed_power_ml/models/gupta_model/apparent_power_data_base.yml"
    parameter_file = "src/pulsed_power_ml/models/gupta_model/parameters.yml"
    training_data_folder = "../training_data/labels_20221202_1peak/"
    validation_data_folder = '../training_data/labels_20221202_1peak_validation/'

    scalers = ["StandardScaler", "MinMaxScaler", "MaxAbsScaler", "RobustScaler", "NoScaler"]

    k_range = list(range(1, 31))

    
    
    for scaler in scalers:
        
        scores = []
        
        for ii in k_range:
            score = evaluate_knn_classifier(parameter_file, 
                                            power_db_file,
                                            training_data_folder,
                                            validation_data_folder,
                                            n_neighb=ii,
                                            scaler=scaler,
                                            makeplot=False)
            scores.append(score)
        
        plt.plot(k_range, scores)
        plt.xlabel("n_neighbors")
        plt.ylabel("accuracy score")
        plt.title("1 peaks, {0}: best n = {1}, best score = {2}".format(scaler,k_range[scores.index(max(scores))] ,max(scores)))
        plt.show()