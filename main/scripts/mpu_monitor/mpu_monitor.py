#!/usr/bin/env python3
import json
import math
import threading

import rclpy
from rclpy.node import Node
from std_msgs.msg import String

import pygame
from pygame.locals import DOUBLEBUF, OPENGL, QUIT, KEYDOWN, K_ESCAPE, K_z
from OpenGL.GL import *
from OpenGL.GLU import *

roll_deg = 0.0
pitch_deg = 0.0
yaw_deg = 0.0
yaw_mode = False
lock = threading.Lock()


class MpuViewerNode(Node):
    def __init__(self):
        super().__init__("mpu_viewer")
        self.create_subscription(String, "/rov/sensors", self.cb, 10)

    def cb(self, msg: String):
        global roll_deg, pitch_deg, yaw_deg
        try:
            data = json.loads(msg.data)
            mpu = data.get("mpu", {})
            angle = mpu.get("angle", [0.0, 0.0, 0.0])

            # assume [roll, pitch, yaw] in radians
            r = math.degrees(float(angle[0]))
            p = math.degrees(float(angle[1]))
            y = math.degrees(float(angle[2]))

            with lock:
                roll_deg = r
                pitch_deg = p
                yaw_deg = y

        except Exception as e:
            self.get_logger().warn(f"Bad sensor frame: {e}")


def resize(width, height):
    if height == 0:
        height = 1
    glViewport(0, 0, width, height)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    gluPerspective(45, width / height, 0.1, 100.0)
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()


def init_gl():
    glShadeModel(GL_SMOOTH)
    glClearColor(0.0, 0.0, 0.0, 0.0)
    glClearDepth(1.0)
    glEnable(GL_DEPTH_TEST)
    glDepthFunc(GL_LEQUAL)
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST)


def draw_text(position, text_string):
    font = pygame.font.SysFont("Courier", 18, True)
    text_surface = font.render(text_string, True, (255, 255, 255, 255), (0, 0, 0, 255))
    text_data = pygame.image.tostring(text_surface, "RGBA", True)
    glRasterPos3d(*position)
    glDrawPixels(text_surface.get_width(), text_surface.get_height(), GL_RGBA, GL_UNSIGNED_BYTE, text_data)


def draw():
    global yaw_mode
    with lock:
        ax = roll_deg
        ay = pitch_deg
        az = yaw_deg

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    glLoadIdentity()
    glTranslatef(0.0, 0.0, -7.0)

    osd = f"pitch: {ay:.2f}, roll: {ax:.2f}"
    if yaw_mode:
        osd += f", yaw: {az:.2f}"
    draw_text((-2, -2, 2), osd)

    if yaw_mode:
        glRotatef(az, 0.0, 1.0, 0.0)
    glRotatef(ay, 1.0, 0.0, 0.0)
    glRotatef(-ax, 0.0, 0.0, 1.0)

    glBegin(GL_QUADS)

    glColor3f(0.0, 1.0, 0.0)
    glVertex3f( 1.0, 0.2,-1.0)
    glVertex3f(-1.0, 0.2,-1.0)
    glVertex3f(-1.0, 0.2, 1.0)
    glVertex3f( 1.0, 0.2, 1.0)

    glColor3f(1.0, 0.5, 0.0)
    glVertex3f( 1.0,-0.2, 1.0)
    glVertex3f(-1.0,-0.2, 1.0)
    glVertex3f(-1.0,-0.2,-1.0)
    glVertex3f( 1.0,-0.2,-1.0)

    glColor3f(1.0, 0.0, 0.0)
    glVertex3f( 1.0, 0.2, 1.0)
    glVertex3f(-1.0, 0.2, 1.0)
    glVertex3f(-1.0,-0.2, 1.0)
    glVertex3f( 1.0,-0.2, 1.0)

    glColor3f(1.0, 1.0, 0.0)
    glVertex3f( 1.0,-0.2,-1.0)
    glVertex3f(-1.0,-0.2,-1.0)
    glVertex3f(-1.0, 0.2,-1.0)
    glVertex3f( 1.0, 0.2,-1.0)

    glColor3f(0.0, 0.0, 1.0)
    glVertex3f(-1.0, 0.2, 1.0)
    glVertex3f(-1.0, 0.2,-1.0)
    glVertex3f(-1.0,-0.2,-1.0)
    glVertex3f(-1.0,-0.2, 1.0)

    glColor3f(1.0, 0.0, 1.0)
    glVertex3f( 1.0, 0.2,-1.0)
    glVertex3f( 1.0, 0.2, 1.0)
    glVertex3f( 1.0,-0.2, 1.0)
    glVertex3f( 1.0,-0.2,-1.0)

    glEnd()


def main():
    global yaw_mode

    rclpy.init()
    node = MpuViewerNode()

    pygame.init()
    pygame.display.set_mode((640, 480), OPENGL | DOUBLEBUF)
    pygame.display.set_caption("ROV MPU Viewer")
    resize(640, 480)
    init_gl()

    clock = pygame.time.Clock()

    try:
        while rclpy.ok():
            rclpy.spin_once(node, timeout_sec=0.0)

            for event in pygame.event.get():
                if event.type == QUIT:
                    return
                if event.type == KEYDOWN and event.key == K_ESCAPE:
                    return
                if event.type == KEYDOWN and event.key == K_z:
                    yaw_mode = not yaw_mode

            draw()
            pygame.display.flip()
            clock.tick(60)
    finally:
        node.destroy_node()
        rclpy.shutdown()
        pygame.quit()


if __name__ == "__main__":
    main()