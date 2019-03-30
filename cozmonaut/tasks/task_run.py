#
# InsertProjectName
# Copyright (c) 2019 The InsertProjectName Contributors
# InsertLicenseText
#


class TaskRun:
    """
    A task to run the Cozmo program.
    """

    def __init__(self, num_cozmos):
        self.num_cozmos = num_cozmos

    def run(self):
        print(f'Running with {self.num_cozmos} Cozmos... NOT IMPLEMENTED')
