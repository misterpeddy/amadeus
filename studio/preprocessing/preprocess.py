r"""Create a TFRecord dataset from audio files.

Usage:
====================
preprocess \
--input_path=/path/to/wavs/*wav,/path/to/mp3s/*mp3 \
--output_path=/path/to/output.tfrecord \

"""

from absl import app
from absl import flags
from ddsp.training.data_preparation.prepare_tfrecord_lib import prepare_tfrecord
import tensorflow.compat.v2 as tf

FLAGS = flags.FLAGS

flags.DEFINE_list(
    'input_path', None, 
    'List of filepatterns to glob for input audio files.')
flags.DEFINE_string(
    'output_path', None,
    'The prefix path to the output TFRecord. Shard numbers will be added to '
    'actual path(s).')
flags.DEFINE_integer(
    'num_shards', None,
    'The number of shards to use for the TFRecord. If None, this number will '
    'be determined automatically.')
flags.DEFINE_integer(
    'sample_rate', 16000,
    'The sample rate to use for the audio.')
flags.DEFINE_integer(
    'frame_rate', 250,
    'The frame rate to use for f0 and loudness features. If set to 0, '
    'these features will not be computed.')
flags.DEFINE_float(
    'example_secs', 4,
    'The length of each example in seconds. Input audio will be split to this '
    'length using a sliding window. If 0, each full piece of audio will be '
    'used as an example.')
flags.DEFINE_float(
    'sliding_window_hop_secs', 1,
    'The hop size in seconds to use when splitting audio into constant-length '
    'examples.')
flags.DEFINE_list(
    'pipeline_options', '--runner=DirectRunner',
    'A comma-separated list of command line arguments to be used as options '
    'for the Beam Pipeline.')

def main(argv):
  input_paths = []
  for filepattern in FLAGS.input_path:
    input_paths.extend(tf.io.gfile.glob(filepattern))

  prepare_tfrecord(
      input_paths,
      FLAGS.output_path,
      num_shards=FLAGS.num_shards,
      sample_rate=FLAGS.sample_rate,
      frame_rate=FLAGS.frame_rate,
      window_secs=FLAGS.example_secs,
      hop_secs=FLAGS.sliding_window_hop_secs,
      pipeline_options=FLAGS.pipeline_options)

if __name__ == '__main__':
  flags.mark_flag_as_required('input_path')
  flags.mark_flag_as_required('output_path')
  app.run(main)
