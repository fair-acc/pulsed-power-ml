import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestClassifier
from sklearn import metrics
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
# Set random seed
seed = 42

################################
########## DATA PREP ###########
################################


# Load in the data
df = pd.read_csv("data/raw/wine_quality.csv")

# Split into train and test sections
y = df.pop("quality")
y = (y > 6)     # is it good wine
X_train, X_test, y_train, y_test = train_test_split(df, y, test_size=0.2, random_state=seed)

#################################
########## MODELLING ############
#################################

# Fit a model on the train section
clf = RandomForestClassifier(max_depth=6, random_state=seed)
clf.fit(X_train, y_train)

# Report training set score
train_score = clf.score(X_train, y_train) * 100
# Report test set score
test_score = clf.score(X_test, y_test) * 100

# Write scores to a file
with open("metrics.txt", 'w') as outfile:
        outfile.write("Training Accuracy: %2.1f%%\n" % train_score)
        outfile.write("Test Accuracy: %2.1f%%\n" % test_score)

# Calc ROC
y_pred = clf.predict_proba(X_test)
fpr, tpr, thresholds = metrics.roc_curve(y_test, y_pred[:, 1])
pd.DataFrame([fpr, tpr]).transpose().rename({0: 'fpr', 1: 'tpr'}, axis=1).to_csv("roc.csv")
