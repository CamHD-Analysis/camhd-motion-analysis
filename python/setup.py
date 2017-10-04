from setuptools import setup

setup(name='camhd_motion_analysis',
      version='0.1',
      description='',
      url='http://github.com/amarburg/camhd_motion_analysis',
      author='Aaron Marburg',
      author_email='amarburg@apl.washington.edu',
      license='MIT',
      packages=['camhd_motion_analysis'],
      scripts=['region_analysis.py', 'rq_job_injector.py'],
      install_requires=['py-cpuinfo'],
      zip_safe=False)
