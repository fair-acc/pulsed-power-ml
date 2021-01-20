import pandas as pd
from datetime import datetime
import tensorflow as tf
from sklearn.model_selection import train_test_split
from tensorflow import feature_column
from tensorflow.keras import layers

# Set random seed
seed = 42

logdir = "../../model/logs/" + datetime.now().strftime("%Y%m%d-%H%M%S") + "/"


def df_to_dataset(df_in, shuffle=True, batchsize=32):
    dataframe = df_in.copy()
    labels = dataframe.pop('quality')
    ds = tf.data.Dataset.from_tensor_slices((dict(dataframe), labels))
    if shuffle:
        ds = ds.shuffle(buffer_size=len(dataframe))
    ds = ds.batch(batchsize)
    return ds


# Load in the data
cols = ['fixed_acidity', 'volatile_acidity', 'citric_acid', 'residual_sugar',
        'chlorides', 'free_sulfur_dioxide', 'total_sulfur_dioxide', 'density',
        'pH', 'sulphates', 'alcohol', 'quality']

df = pd.read_csv("../../data/raw/wine_quality.csv", names=cols, header=0)

train, test = train_test_split(df, test_size=0.2)
train, val = train_test_split(train, test_size=0.2)

batch_size = 5
train_ds = df_to_dataset(train, batchsize=batch_size)
val_ds = df_to_dataset(val, shuffle=False, batchsize=batch_size)
test_ds = df_to_dataset(test, shuffle=False, batchsize=batch_size)

for feature_batch, label_batch in train_ds.take(1):
  print('Every feature:', list(feature_batch.keys()))
  print('A batch of alcohol:', feature_batch['alcohol'])
  print('A batch of targets:', label_batch)

#feature_batch = train_ds.take(1)

feature_columns = []
for header in list(feature_batch.keys()):
    feature_columns.append(feature_column.numeric_column(header))
feature_layer = tf.keras.layers.DenseFeatures(feature_columns)


tb_callback = tf.keras.callbacks.TensorBoard(
    log_dir=logdir, histogram_freq=0, write_graph=True,
    update_freq='epoch', profile_batch=2)

model = tf.keras.Sequential([
    feature_layer,
    layers.Dense(256, activation='relu'),
    layers.Dense(32, activation='relu'),
    layers.Dropout(.1),
    layers.Dense(6)
])

model.compile(optimizer='adam',
              loss='mse',
              metrics=['accuracy'])

model.fit(train_ds,
          validation_data=val_ds,
          epochs=50,
          callbacks=[tb_callback])
