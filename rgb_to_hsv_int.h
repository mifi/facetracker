struct rgb_color {
    unsigned char r, g, b;    /* Channel intensities between 0 and 255 */
};

struct hsv_color {
    unsigned char hue;        /* Hue degree between 0 and 255 */
    unsigned char sat;        /* Saturation between 0 (gray) and 255 */
    unsigned char val;        /* Value between 0 (black) and 255 */
};


struct hsv_color rgb_to_hsv(struct rgb_color rgb);
