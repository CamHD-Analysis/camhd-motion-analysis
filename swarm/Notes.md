

Create new project "camhd-motion-analysis-swarm" on GCP

Add bitnami/redis from Cloud Launcher.   Standard configuration

gloud init, create configuration

gcloud auth


docker-machine create swarm-manager -d google --google-machine-type g1-small --google-zone us-central1-a --google-tags swarm-cluster --google-project camhd-motion-analysis-swarm --swarm-master

## Create boot image

gcloud compute instance create worker-template \
            --machine-type n1-standard-4 \
            --image-family  \
            --image-project  \
            --boot-disk-size 10GB



gcloud compute instance-templates create worker-template \
            --machine-type n1-standard-4 \
            --image-family  \
            --image-project  \
            --boot-disk-size 10GB


redis://:q3xUPXYE@redis-vm/0


gcloud compute instance-groups managed create worker-swarm \
  --base-instance-name worker \
  --size 2 \
  --template \
  --zone us-central1-a
