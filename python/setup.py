from setuptools import setup

setup(name='camhd_motion_analysis',
      version='0.2',
      description='',
      url='http://github.com/amarburg/camhd_motion_analysis',
      author='Aaron Marburg',
      author_email='amarburg@apl.washington.edu',
      license='MIT',
      packages=['camhd_motion_analysis'],
      scripts=['apps/recent_injector.py', 'apps/job_injector.py'],
      install_requires=['celery','py-cpuinfo','minio','python-decouple'],
        extras_require={
            'test':  ["pytest-docker"],
        },
      zip_safe=False)
