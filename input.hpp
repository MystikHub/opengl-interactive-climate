#ifndef CAMERA_HPP
#define CAMERA_HPP

class Input {
public:
    

private:
    int mouse_move_threshold = 1;
    int threshLeft = Camera::horizontal_center - mouse_move_threshold;
    int threshRight = Camera::horizontal_center + mouse_move_threshold;
    int threshTop = Camera::vertical_center + mouse_move_threshold;
    int threshBottom = Camera::vertical_center - mouse_move_threshold;
    int prevX = Camera::horizontal_center;
    int prevY = Camera::vertical_center;
};

#endif