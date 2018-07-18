
from camhd_motion_analysis import process_file

import pytest

from check_json_file import check_json_file

mov_path = 'RS03ASHS/PN03B/06-CAMHDA301/2018/07/01/CAMHDA301-20180701T001500.mov'


def test_process_file_output_to_file():

    destination = "test_tmp/test_process_file_with_output.json"

    result = process_file( mov_path, destination,
                stop=20 )

    ## Todo.  Check contents of file
    check_json_file("test_tmp/test_process_file_with_output.json")
