
from pathlib import Path
import json


def check_json(result):

    assert(result)

    assert( 'frameStats' in result )
    assert( 'movie' in result )
    assert( 'contents' in result )

def check_json_file(path):

    p = Path(path)

    assert( p.is_file() )

    with open( p ) as f:
        result = json.load(f)
        check_json(result)
