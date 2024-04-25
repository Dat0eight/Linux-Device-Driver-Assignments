/* This is driver that handle button state, snake game logic,  5110 nokia LCD
*------------------------------ALL IN ONE------------------------------
 */

#include "driver.h"

#define DRIVER_AUTHOR "wangdat"
#define DRIVER_DESC   "Snake game - Using Device Tree and SPI protocol technique"
#define DRIVER_VERSION "1.0"
#define DRIVER_LICENSE "GPL"

static const struct of_device_id snake_game_dev_ids[] = {
    { .compatible = "nokia, snake game", },
    { /* sentinel */ }
};

MODULE_DEVICE_TABLE(of, snake_game_dev_ids);

static struct spi_driver snake_game_driver = {
	.probe = game_probe,
	.remove = game_remove,
	.driver = {
		.name = "snake game driver",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(snake_game_dev_ids),
	},
};

module_spi_driver(snake_game_driver);

static int game_probe(struct spi_device *spi)
{
    struct nokia_lcd *lcd = NULL;
	struct device *game_device = &spi->dev;
    count = 0;

	printk(KERN_INFO "Start LCD ... !\n");
	lcd = devm_kzalloc(&spi->dev, sizeof(*lcd), GFP_KERNEL);
	if(lcd == NULL)
	{
		printk(KERN_ERR "Allocate device failure\n");
		return -1;
	}
	lcd->reset = gpiod_get(game_device, "reset", GPIOD_OUT_HIGH);
	gpiod_set_value(lcd->reset, 1);
	lcd->dc = gpiod_get(game_device, "dc", GPIOD_OUT_HIGH);
	lcd->led = gpiod_get(game_device, "led", GPIOD_OUT_HIGH);
	lcd->spi = spi;
	spi_set_drvdata(spi, lcd);
	lcd->spi->mode = SPI_MODE_3;
	lcd->spi->bits_per_word = 8;
	lcd->spi->max_speed_hz = 1000000;
	if (spi_setup(lcd->spi) < 0)
	{
		printk(KERN_ERR "SPI setup failed\n");
		goto rm_lcd;
	}

    lcd->head.x = 7; //{7, 3};       //starting position of the snake              
    lcd->head.y = 3;               
    lcd->dir.x = 1; //{1, 0};        //starting movement
    lcd->dir.y = 0; 
    lcd->berry.x = 4; //{4, 3};      //starting position of the first berry
    lcd->berry.y = 3; 

    lcd_init(lcd);

    lcd_clear(lcd);

    lcd_goto_XY(lcd, 0, 1);
    lcd_send_string(lcd, "--Snake Game--");

    draw_border(SCREEN_X0_COORDINATE, SCREEN_Y0_COORDINATE, 
                SCREEN_X1_COORDINATE, SCREEN_Y1_COORDINATE, lcd);

    printk(KERN_INFO "Init successful\n");
        
    lcd_gpiod_get(game_device); //Configuration the GPIO direction and an initial value

    btn_direction_input(); //direction input button

    state_of_btn ();    //print state of button
    
    set_btn_irq();  //For GPIO descriptor mapped to IRQ
    
    /* setup the timer to call game_timer_callback */
    timer_setup(&lcd->game_speed_timer, game_timer_callback, 0);
  
    lcd->game_speed_timer.expires = jiffies + msecs_to_jiffies(GAME_SPEED);  
    add_timer(&lcd->game_speed_timer);

    pr_info("This is end probe\n");
    return 0;

rm_lcd:
	gpiod_put(lcd->dc);
	gpiod_put(lcd->led);
	gpiod_put(lcd->reset);
	return -1;
}

static int game_remove(struct spi_device *spi)
{
    struct nokia_lcd *lcd = spi_get_drvdata(spi);
	if(lcd == NULL)
	{
		printk(KERN_ERR "Cannot get driver data\n");
	}
	else
	{
    lcd_clear(lcd);
    gpiod_put(lcd->dc);
	gpiod_put(lcd->led);
	gpiod_put(lcd->reset);
    }
    del_timer(&lcd->game_speed_timer);
    free_gpiod_btn();
    free_irq_btn();
    pr_info("misc_register exit done!!!\n");
    return 0;
}

static irqreturn_t left_btn_irq_handler(int irq, void *dev_id){
    int state;
    pr_info("This is left_btn_irq_handler \n");
    state = gpiod_get_value(left_btn);
    pr_info("State of left button: %d \n",state);
    direction = DIRECTION_LEFT;
    pr_info("snake turn left, direction: %d\n", direction);
    pr_info("------------------------- \n");
    return IRQ_HANDLED;
}

static irqreturn_t right_btn_irq_handler(int irq, void *dev_id){
    int state;
    pr_info("This is right_btn_irq_handler \n");
    state = gpiod_get_value(right_btn);
    pr_info("State of right button: %d \n",state);
    direction = DIRECTION_RIGHT;
    pr_info("snake turn right, direction: %d\n", direction);
    pr_info("------------------------- \n");
    return IRQ_HANDLED;
}

static irqreturn_t up_btn_irq_handler(int irq, void *dev_id){
    int state;
    pr_info("This is up_btn_irq_handler \n");
    state = gpiod_get_value(up_btn);
    pr_info("State of up button: %d \n",state);
    direction = DIRECTION_UP;
    pr_info("snake go up, direction: %d\n", direction);
    pr_info("------------------------- \n");
    return IRQ_HANDLED;
}

static irqreturn_t down_btn_irq_handler(int irq, void *dev_id){
    int state;
    pr_info("This is down_btn_irq_handler \n");
    state = gpiod_get_value(down_btn);
    pr_info("State of down button: %d \n",state);
    direction = DIRECTION_DOWN;
    pr_info("snake go down, direction: %d\n", direction);
    pr_info("------------------------- \n");
    return IRQ_HANDLED;
}

static irqreturn_t reset_btn_irq_handler(int irq, void *dev_id){
    int state;

    pr_info("This is reset_btn_irq_handler \n");
    state = gpiod_get_value(reset_btn);
    pr_info("State of reset button: %d \n",state);
    direction = DIRECTION_RESET;
    pr_info("pressed reset, direction: %d\n", direction);
    pr_info("------------------------- \n");

    return IRQ_HANDLED;
}

static int lcd_gpiod_get(struct device *dev){
        left_btn = gpiod_get(dev, "leftButton", GPIOD_IN);
    if (IS_ERR(left_btn)){
        printk(KERN_ALERT "Failed to get left button\n");
        return -ENOENT;
    }

    right_btn = gpiod_get(dev, "rightButton", GPIOD_IN);
    if (IS_ERR(right_btn)){
        printk(KERN_ALERT "Failed to get right button\n");
        return -ENOENT;
    }

    up_btn = gpiod_get(dev, "upButton", GPIOD_IN);
    if (IS_ERR(up_btn)){
        printk(KERN_ALERT "Failed to get up button\n");
        return -ENOENT;
    }

    down_btn = gpiod_get(dev, "downButton", GPIOD_IN);
    if (IS_ERR(down_btn)){
        printk(KERN_ALERT "Failed to get down button\n");
        return -ENOENT;
    }
    
    reset_btn = gpiod_get(dev, "resetButton", GPIOD_IN);
    if (IS_ERR(reset_btn)){
        printk(KERN_ALERT "Failed to get reset button\n");
        return -ENOENT;
    }
    return 0;
}

static void btn_direction_input(void){
    gpiod_direction_input(left_btn);
    gpiod_direction_input(right_btn);
    gpiod_direction_input(up_btn);
    gpiod_direction_input(down_btn);
    gpiod_direction_input(reset_btn);
}

static void state_of_btn (void){
    pr_info("left-button state: %d \n", gpiod_get_value(left_btn));
    pr_info("right-button state: %d \n", gpiod_get_value(right_btn));
    pr_info("up-button state: %d \n", gpiod_get_value(up_btn));
    pr_info("down-button state: %d \n", gpiod_get_value(down_btn));
    pr_info("reset-button state: %d \n", gpiod_get_value(reset_btn));
}

static int set_btn_irq(void){
    left_btn_irq = gpiod_to_irq(left_btn);
    pr_info("left_btn_irq number = %d\n", left_btn_irq);
    if ((request_irq(left_btn_irq, left_btn_irq_handler,
                        IRQF_TRIGGER_FALLING,
                        "button_misc_dev", NULL)) != 0){
        printk(KERN_ALERT "Failed to register left button IRQ\n");
        return -ENOENT;
    }

    right_btn_irq = gpiod_to_irq(right_btn);
    pr_info("right_btn_irq number = %d\n", right_btn_irq);
    if ((request_irq(right_btn_irq, right_btn_irq_handler,
                        IRQF_TRIGGER_FALLING,
                        "button_misc_dev", NULL)) != 0){
        printk(KERN_ALERT "Failed to register right button IRQ\n");
        return -ENOENT;
    }

    up_btn_irq = gpiod_to_irq(up_btn);
    pr_info("up_btn_irq number = %d\n", up_btn_irq);
    if ((request_irq(up_btn_irq, up_btn_irq_handler,
                        IRQF_TRIGGER_FALLING,
                        "button_misc_dev", NULL)) != 0){
        printk(KERN_ALERT "Failed to register up button IRQ\n");
        return -ENOENT;
    }

    down_btn_irq = gpiod_to_irq(down_btn);
    pr_info("down_btn_irq number = %d\n", down_btn_irq);
    if ((request_irq(down_btn_irq, down_btn_irq_handler,
                        IRQF_TRIGGER_FALLING,
                        "button_misc_dev", NULL)) != 0){
        printk(KERN_ALERT "Failed to register down button IRQ\n");
        return -ENOENT;
    }

    reset_btn_irq = gpiod_to_irq(reset_btn);
    pr_info("reset_btn_irq number = %d\n", reset_btn_irq);
    if ((request_irq(reset_btn_irq, reset_btn_irq_handler,
                        IRQF_TRIGGER_FALLING,
                        "button_misc_dev", NULL)) != 0){
        printk(KERN_ALERT "Failed to register reset button IRQ\n");
        return -ENOENT;
    }
    return 0;
}

static void free_gpiod_btn(void){
    gpiod_put(left_btn);
    gpiod_put(right_btn);
    gpiod_put(up_btn);
    gpiod_put(down_btn);
    gpiod_put(reset_btn);
}

static void free_irq_btn(void){
    free_irq(left_btn_irq, NULL);
    free_irq(right_btn_irq, NULL);
    free_irq(up_btn_irq, NULL);
    free_irq(down_btn_irq, NULL);
    free_irq(reset_btn_irq, NULL);
}

void lcd_init(struct nokia_lcd *lcd)
{
	lcd_reset(lcd);
	lcd_write_one_byte(lcd, LCD_ADDFUNCTIONSET, LCD_COMMAND); // Using the additional commands, horizontal addressing
	lcd_write_one_byte(lcd, LCD_SETVOP | LCD_SETVOP2, LCD_COMMAND);
	lcd_write_one_byte(lcd, LCD_FUNCTIONSET, LCD_COMMAND); // Using the basic command
	lcd_write_one_byte(lcd, DISPLAYCONTROL | LCD_DISPLAYNORMAL, LCD_COMMAND);
	lcd_write_one_byte(lcd, LCD_ADDFUNCTIONSET, LCD_COMMAND);
	lcd_write_one_byte(lcd, LCD_SETXADDR | LCD_SETYADDR, LCD_COMMAND);
	lcd_write_one_byte(lcd, LCD_FUNCTIONSET, LCD_COMMAND);
}

void lcd_reset(struct nokia_lcd *lcd)
{
	gpiod_set_value(lcd->reset, GPIOD_CMD_VALUE);
	mdelay(20);
	gpiod_set_value(lcd->reset, GPIOD_DATA_VALUE);
}

void lcd_clear(struct nokia_lcd *lcd){
	uint32_t pixel;
	for(pixel = 0; pixel < 504; pixel++)    // 504 = screen_height * screen_width / 8
	{
		lcd_write_one_byte(lcd, 0x00, GPIOD_DATA_VALUE);
	}
	lcd_goto_XY(lcd, 0, 0);
}

void lcd_write_one_byte(struct nokia_lcd *lcd, uint8_t data, uint8_t mode)
{
	if(mode == LCD_COMMAND)
		gpiod_set_value(lcd->dc, GPIOD_CMD_VALUE);
	else if(mode == LCD_DATA)
		gpiod_set_value(lcd->dc, GPIOD_DATA_VALUE);
	spi_write(lcd->spi, &data, GPIOD_DATA_VALUE);
}

void lcd_send_string(struct nokia_lcd *lcd, uint8_t *s)
{
	gpiod_set_value(lcd->dc, GPIOD_DATA_VALUE);
	while(*s)
	{
		lcd_send_char(lcd, *s++);
	}
}

void lcd_send_char(struct nokia_lcd *lcd, uint8_t data)
{
	spi_write(lcd->spi, font6x8[data-32], 6);
}

void lcd_draw_into_XY(struct nokia_lcd *lcd, uint8_t *s, uint8_t X, uint8_t Y){
    lcd_write_one_byte(lcd, LCD_SETYADDR | Y, LCD_COMMAND);
	lcd_write_one_byte(lcd, LCD_SETXADDR | X * 6, LCD_COMMAND);

    lcd_send_string(lcd, s);
}

void lcd_goto_XY(struct nokia_lcd *lcd, uint8_t X, uint8_t Y)
{
	lcd_write_one_byte(lcd, LCD_SETYADDR | Y, LCD_COMMAND);
	lcd_write_one_byte(lcd, LCD_SETXADDR | X*6, LCD_COMMAND);
}

static u8 create_random_number(u8 max_number)
{
	u8 random_number;
	get_random_bytes(&random_number, sizeof(random_number));
	return random_number % max_number;
}

void update_snake (struct nokia_lcd *lcd){
        //update snake
    int i, j;

    for (i = score + size_starting_snake; i > 0; i--){
        snake[i] = snake[i - 1];
    }

    snake[0] = lcd->head;

    lcd->head.x += lcd->dir.x;
    lcd->head.y += lcd->dir.y;

    //the snake eats berry
    if (lcd->berry.y == lcd->head.y && lcd->berry.x == lcd->head.x){
        score += 1;
        // sprintf(score_message, "[Score: %d]", score);
        lcd->berry.x = create_random_number(13);
        lcd->berry.y = create_random_number(5);
    }

    //draw a berry and a snake
    lcd_clear(lcd);
    lcd_draw_into_XY(lcd, "0", lcd->head.x, lcd->head.y);
    lcd_draw_into_XY(lcd, "@", lcd->berry.x, lcd->berry.y);
    for (j = 0; j < score + size_starting_snake; j++){
        lcd_draw_into_XY(lcd, "o", snake[j].x, snake[j].y);
    }
}

void draw_border(int x, int y, int width, int height, struct nokia_lcd *lcd) {
    int i;

    // top row line
    for (i = 0; i <= width; i++) {
        lcd_draw_into_XY(lcd, "*", x + i, y);
    }
    
    // vertical line
    for (i = 1; i <= height; i++) {
        lcd_draw_into_XY(lcd, "*", x, y + i);
        lcd_draw_into_XY(lcd, "*", x + width, y + i);
    }

    // bottom row line
    for (i = 0; i < width; i++) {
        lcd_draw_into_XY(lcd, "*", x + i, y + height);
    }
}

void game_timer_callback(struct timer_list *data){
    /* game loop */
    struct nokia_lcd *lcd;

    lcd = container_of(data, struct nokia_lcd, game_speed_timer);
    pr_info("Timer Callback function Called [%d]\n",count++);
    
    update_snake(lcd);
    process_input(lcd);

    if (border_collision(lcd->head)!= true){
        /*
       Re-enable timer. Because this function will be called only first time. 
       If we re-enable this will work like periodic timer. 
        */
        mod_timer(&lcd->game_speed_timer, jiffies + msecs_to_jiffies(GAME_SPEED));
    } else {
        lcd_clear(lcd);
        lcd_draw_into_XY(lcd,"--GO: wall--" ,0, 1);
        sprintf(lcd->message, "[Score: %d]", score);
        lcd_draw_into_XY(lcd, lcd->message, 1, 3);
        del_timer(&lcd->game_speed_timer);

    }

    if (snake_collision(lcd->head) != true){
        mod_timer(&lcd->game_speed_timer, jiffies + msecs_to_jiffies(GAME_SPEED));
    } else {
        lcd_clear(lcd);
        lcd_draw_into_XY(lcd,"--GO: self--" ,0, 4);
        sprintf(lcd->message, "[Score: %d]", score);
        lcd_draw_into_XY(lcd, lcd->message, 1, 3);
        del_timer(&lcd->game_speed_timer);

    }
    
    pr_info("This after func\n");
}

void process_input(struct nokia_lcd *lcd){
    int is_pressing = direction;

    switch (is_pressing){
        case DIRECTION_LEFT:
            if(lcd->dir.x == 1) return;
            lcd->dir.x = -1;
            lcd->dir.y = 0;  
            break;

        case DIRECTION_RIGHT:
            if(lcd->dir.x == -1) return;
            lcd->dir.x = 1;
            lcd->dir.y = 0;
            break;

        case DIRECTION_UP:
            if(lcd->dir.y == 1) return;
            lcd->dir.x = 0;
            lcd->dir.y = -1;
            break;

        case DIRECTION_DOWN:
            if(lcd->dir.y == -1) return;
            lcd->dir.x = 0;
            lcd->dir.y = 1;

        case DIRECTION_RESET:
            return;
    }
}

bool border_collision(struct snake_vector head_point){
    if (head_point.x < 0 || head_point.y < 0 || 
        head_point.x > 13 || head_point.y > 5){
        return true;    //having a collision
    } return false;
}

bool snake_collision(struct snake_vector head_point){
    int i;
    for (i =0; i < (score + size_starting_snake); i++){
        if (head_point.x == snake[i].x && head_point.y == snake[i].y){
        return true;    //having a collision
        }
    } 
    return false;
}

MODULE_LICENSE(DRIVER_LICENSE);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);