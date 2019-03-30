#
# InsertProjectName
# Copyright (c) 2019 The InsertProjectName Contributors
# InsertLicenseText
#

import asyncio

import cozmo

import facelib


class TaskRun:
    """
    A task to run the Cozmo program.
    """

    @staticmethod
    def fun():
        print('called')

    def __init__(self, num_cozmos):
        self.num_cozmos = num_cozmos

        # Set up face recognizer
        self.face_recognizer = facelib.FaceRecognizer()
        self.face_recognizer.listen(self.fun)
        self.face_recognizer.poll()

    def run(self):
        # Get an event loop
        loop = asyncio.get_event_loop()

        # Coroutines we must wait on as part of the task
        coroutines = []

        # Connect the Cozmos asynchronously
        # For each Cozmo, we spin up a coroutine and add it to the list above
        for i in range(self.num_cozmos):
            conn = cozmo.connect_on_loop(loop)
            coroutines.append(asyncio.ensure_future(self._client_program(conn), loop=loop))

        # Wait on all the established coroutines
        loop.run_until_complete(asyncio.gather(*coroutines))

    @staticmethod
    async def _client_program(conn: cozmo.conn.CozmoConnection) -> None:
        """
        The client program.

        :param conn: The SDK connection
        """

        # Wait for the robot to be ready, and then get it
        robot: cozmo.robot.Robot = await conn.wait_for_robot()

        # TODO
        await robot.say_text('hello world').wait_for_completed()


# Make Cozmo stay on the charger when we connect
cozmo.robot.Robot.drive_off_charger_on_connect = False
