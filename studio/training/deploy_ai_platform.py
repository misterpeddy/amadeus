r"""Script for building the training image and submitting AI Platform job.
Usage:
================================================================================
python deploy_ai_platform.py \
--data_path=gs://ddsp_training/data \
--save_path=gs://ddsp_training/model \
--config_path=/local/path/single_vm_config.yaml
"""

import datetime
import os
import subprocess

from absl import app
from absl import flags
from google.cloud import storage

# Set workload identity
service_account_path = '/home/peddy/repos/amadeus/gcs_sa.json'
if os.path.exists(sa_path):
  os.environ['GOOGLE_APPLICATION_CREDENTIALS'] = service_account_path 

# Required flags
flags.DEFINE_string('data_path', None,
                    'Path where the dataset for training is saved')
flags.DEFINE_string('save_path', None, 
                    'Path where checkpoints and summary events will be saved '
                    'during training and evaluation.')
flags.DEFINE_string('config_path', None,
                    'Configuration file for AI Platform training')

# Flags with inferred defaults
flags.DEFINE_string('restore_path', '',
                    'Path from which checkpoints will be restored before '
                    'training. Can be different than the save_path.')
flags.DEFINE_string('project_id', '', 'GCP Project ID')
flags.DEFINE_string('region', '', 'GCP region for running the training')

# Flags with pre-set defaults
flags.DEFINE_string('batch_size', '16', 'Batch size')
flags.DEFINE_string('learning_rate', '0.0001', 'Learning rate')
flags.DEFINE_string('num_steps', '40000', 'Number of training steps')
flags.DEFINE_string('steps_per_summary', '300', 'Steps per summary')
flags.DEFINE_string('steps_per_save', '300', 'Steps per save')
flags.DEFINE_string('early_stop_loss_value', '5',
                    'Early stopping. When the total_loss reaches below this '
                    'value training stops.')

FLAGS = flags.FLAGS


def check_project_id(project_id):
  check_command = 'gcloud projects describe ' + project_id
  if not subprocess.getoutput(check_command).startswith('createTime'):
    raise ValueError('Project ID ' + project_id + ' was invalid.')
  return project_id

def get_project_id():
  project_id = subprocess.getoutput('gcloud config get-value project')
  if project_id == '(unset)':
    project_id = input('Project ID: ')
    project_id = check_project_id(project_id)
  return project_id

def get_region():
  region = subprocess.getoutput('gcloud config get-value compute/region')
  if region == '(unset)':
    raise ValueError('Could not infer GCP Region (compute/region).')
  return region

def build_args():
  """Builds args for ddsp executable."""

  if FLAGS.project_id:
    project_id = check_project_id(FLAGS.project_id)
  else:
    project_id = get_project_id()
  image_uri = 'gcr.io/' + project_id + '/amadeus-train:latest'

  time_diff = datetime.datetime.now() - datetime.datetime(1970, 1, 1)
  uid = int((time_diff).total_seconds())
  job_name = f'training_job_{uid}'

  if not FLAGS.region:
    region = get_region()

  args = {'data_path': FLAGS.data_path,
          'save_path': FLAGS.save_path,
          'restore_path': FLAGS.restore_path if FLAGS.restore_path else FLAGS.save_path,
          'config_path': FLAGS.config_path,
          'image_uri': image_uri,
          'job_name': job_name,
          'region': region,
          'batch_size': FLAGS.batch_size,
          'learning_rate': FLAGS.learning_rate,
          'num_steps': FLAGS.num_steps,
          'steps_per_save': FLAGS.steps_per_save,
          'steps_per_summary': FLAGS.steps_per_summary,
          'early_stop_loss_value': FLAGS.early_stop_loss_value}
  return args


def build_image(args):
  """Builds the docker image."""
  build_command = f'docker build -f Dockerfile -t {args["image_uri"]} ./'
  os.system(build_command)
  print('Docker image built')


def push_image(args):
  """Pushes the docker image on Google Cloud Registry."""
  pushing_image = f'docker push {args["image_uri"]}'
  os.system(pushing_image)
  print('Image pushed to Google Cloud Registry')


def submit_job(args):
  """Submits the job on AI Platform."""

  submitting_job = (
      'gcloud ai-platform jobs submit training'
      f' {args["job_name"]}'
      f' --region={args["region"]}'
      f' --master-image-uri={args["image_uri"]}'
      f' --config={args["config_path"]}'
      f' -- --save_path={args["save_path"]}'
      f' --restore_path={args["restore_path"]}'
      f' --data_path={args["data_path"]}'
      f' --batch_size={args["batch_size"]}'
      f' --learning_rate={args["learning_rate"]}'
      f' --num_steps={args["num_steps"]}'
      f' --steps_per_summary={args["steps_per_summary"]}'
      f' --steps_per_save={args["steps_per_save"]}'
      f' --early_stop_loss_value={args["early_stop_loss_value"]}')

  os.system(submitting_job)
  print('Job submitted to AI Platform')


def enable_tensorboard(args):
  """Enables Tensorboard."""
  os.system('gcloud auth login')
  tensorboard_command = f'tensorboard --logdir={args["save_path"]} --port=6066 &'
  os.system(tensorboard_command)
  print('Tensorboard enabled')


def upload_logs(args):
  """Uploads logs to TensorBoard.dev."""
  tensorboard_dev_command = ('tensorboard dev upload ' +
                             f'--logdir={args["save_path"]}' +
                             f' --name \"{args["job_name"]}\"')
  os.system(tensorboard_dev_command)
  print('Logs uploaded to TensorBoard.dev')


def main(unused_argv):
  """Builds and pushes image, submits job and enables TensorBoard."""

  args = build_args()
  build_image(args)
  push_image(args)
  submit_job(args)
  enable_tensorboard(args)
  upload_logs(args)


if __name__ == '__main__':
  flags.mark_flag_as_required('data_path')
  flags.mark_flag_as_required('save_path')
  flags.mark_flag_as_required('config_path')
  app.run(main)
