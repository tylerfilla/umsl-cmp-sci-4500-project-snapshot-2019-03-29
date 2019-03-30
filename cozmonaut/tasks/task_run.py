#
# InsertProjectName
# Copyright (c) 2019 The InsertProjectName Contributors
# InsertLicenseText
#

import asyncio
import facelib

import cozmo

from functools import partial

class TaskRun:
    """
    A task to run the Cozmo program.
    """

    def __init__(self, num_cozmos):
        self.num_cozmos = num_cozmos

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

        # Make a face recognizer for this robot
        face_recognizer = facelib.FaceRecognizer()
        face_recognizer.on_face_show(partial(TaskRun._robot_on_face_show, robot))
        face_recognizer.on_face_hide(partial(TaskRun._robot_on_face_hide, robot))
        face_recognizer.on_face_move(partial(TaskRun._robot_on_face_move, robot))

        face_recognizer.poll()

        while True:
            face_recognizer.poll()

    @staticmethod
    def _robot_on_face_show(robot: cozmo.robot.Robot, fid: int, x: int, y: int, width: int, height: int) -> None:
        print(f'face {fid} show: {x} {y} {width} {height}')

    @staticmethod
    def _robot_on_face_hide(robot: cozmo.robot.Robot, fid: int, x: int, y: int, width: int, height: int) -> None:
        print(f'face {fid} hide: {x} {y} {width} {height}')

    @staticmethod
    def _robot_on_face_move(robot: cozmo.robot.Robot, fid: int, x: int, y: int, width: int, height: int) -> None:
        print(f'face {fid} move: {x} {y} {width} {height}')


# Make Cozmo stay on the charger when we connect
cozmo.robot.Robot.drive_off_charger_on_connect = False
