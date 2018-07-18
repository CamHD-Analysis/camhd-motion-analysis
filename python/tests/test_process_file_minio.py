
from camhd_motion_analysis import process_file

import pytest

from check_json_file import check_json_file

from minio import Minio
from minio.error import ResponseError
from urllib.parse import urlparse


mov_path = 'RS03ASHS/PN03B/06-CAMHDA301/2018/07/01/CAMHDA301-20180701T001500.mov'


from requests.exceptions import (
    ConnectionError,
)

def is_responsive(url):
    """Check if something responds to ``url``."""
    o = urlparse(url)

    try:
        client = Minio(o.netloc,
                        access_key='camhd',
                        secret_key='camhdcamhd',
                        secure=False)

        buckets = client.list_buckets()

        print(buckets)

        # No exception == working
        return True
    except ResponseError:
        return False

@pytest.fixture(scope='session')
def minio_server(docker_ip, docker_services):
    """Ensure that "some service" is up and responsive."""
    http_url = 's3://%s:%d/' % (
        docker_ip,
        docker_services.port_for('minio', 9000),
    )

    print("Checking url %s" % http_url)

    docker_services.wait_until_responsive(
       timeout=30.0, pause=1,
       check=lambda: is_responsive(http_url)
    )
    return http_url


def test_process_file_output_to_minio(minio_server):

    result = process_file( mov_path, destination=minio_server + "testbucket/test_process_file_output_to_minio.json",
                stop=20 )

    ## TODO.  Check contents of file

    # result = frame_stats( mov_path, 1000 )
    #
    # assert( 'frameStats' in result )
    # assert( 'movie' in result )
    # assert( 'contents' in result )
