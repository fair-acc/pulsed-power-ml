"""
This module implements a kNN-classifier with the tensorflow API
"""
import tensorflow as tf
from tensorflow import keras


class TFKNeighborsClassifier(keras.Model):

    def __init__(self,
                 n_neighbors: tf.Tensor = tf.constant(5, dtype=tf.int32),
                 name: str = "TFKNeighborsClassifier",
                 **kwargs) -> None:
        """
        Instantiation method for the TensorFlow-implementation of a KNN classifier algorithm.

        Parameters
        ----------
        n_neighbors
            Number of nearest neighbors to take into account.
        name
            Name of the classifier
        kwargs
            key word arguments for keras.Model
        """
        super(TFKNeighborsClassifier, self).__init__(name=name, **kwargs)

        self.n_neighbors = n_neighbors

    def fit(self, X: tf.Tensor, y: tf.Tensor) -> None:
        """
        Simple method to "train" the model.

        Parameters
        ----------
        X
            Tensor containing the feature vectors
        y
            Tensor containing the labels (one-hot-encoded labels with length=N+1).
        """
        self.training_data_features = X
        self.training_data_labels = y
        return

    def call(self, input: tf.Tensor) -> tf.Tensor:
        """
        Method to use this model for inference.

        Parameters
        ----------
        input
            Tensor consisting of one feature vector.

        Returns
        -------
        label
            Tensor containing the classification result.
        """
        difference_vectors = self.training_data_features - input
        distances = tf.norm(difference_vectors, axis=1)

        k_smallest_distances, k_smallest_indices = tf.math.top_k(
            input=distances * -1,
            k=self.n_neighbors,
            name="FindKNearestNeighbors"
        )

        print(k_smallest_indices)

        label_tensor = tf.gather(
            params=self.training_data_labels,
            indices=k_smallest_indices
        )

        # weight classes with distances (similar to scikit learns weights="distance" weighting)
        # ToDo: This needs to be a tf.matmul (I guess)
        weighted_label_tensor = label_tensor / k_smallest_distances

        # Determine the class
        result_index = tf.argmax(tf.reduce_sum(weighted_label_tensor, axis=1))

        # Create result tensor (one-hot encoded w/ length=N+1)
        result_tensor = tf.one_hot(indices=result_index,
                                   depth=self.training_data_labels.shape[1])

        return result_tensor


if __name__ == "__main__":
    model = TFKNeighborsClassifier()

    train_x = tf.random.uniform(shape=tf.constant((20,5)), dtype=tf.float32)
    train_y = tf.one_hot(indices=tf.constant((1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2),
                                             dtype=tf.int32),
                         depth=4)

    test_x = tf.random.uniform(shape=[5],
                               dtype=tf.float32)

    model.fit(train_x, train_y)

    y = model(test_x)

    print(y)