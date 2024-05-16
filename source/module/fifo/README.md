# FIFO module

## FIFO

The fifo module offers a structure to handle storing and accessing data inside a buffer. The initialization function defines the used buffer, size of a single element and the number of elements that can be stored.  
This offers the functionality to not only use it for single bytes but for whole package streams with 100 bytes or more.

## Usage

Filling the fifo:

```c
fifo_t fifo;
uint32_t  buffer[100];

FIFO_RESULT res = fifo_init(&fifo, (uint8_t) sizeof(float), buffer, 100);
if(res != FIFO_OK)
    return;

while(!fifo_is_full(&fifo))
{
    fifo_put32(&fifo, (uint32_t)rand());
}
```

Retrieving the elements:

```c
while(fifo_data_available(&fifo))
{
    uint32_t value = fifo_get32(&fifo);
    // Do something with your value
    printf("%.2f\n", value);
}
```

## Ringbuffer

Ring buffer module that automatically overrides the oldest data.

### Usage

You can get the data by either using `ringbuffer_get` and the index you want or by iterating through the buffer by using `ringbuffer_get_first` and `ringbuffer_get_next`.

Creating the ringbuffer and adding data. In this example a float ringbuffer is used for 1000 elements and is filled with 1000 random values.

```c
ringbuffer_t* buffer = ringbuffer_create(sizeof(float), 1000);

for(int i = 0; i < 1000; i++)
{
     ringbuffer_put(buffer, (float)rand());
}
```

In this example the ringbuffer is read until the end by using the index.

```c
size_t i = 0;
FUNCTION_RETURN_T ret;
do
{
 float value;
 ret = ringbuffer_get(buffer, &value, index);
 if(ret != FUNCTION_RETURN_OK)
     break;
 // Do something with your value
 printf("%.2f\n", value);
 i++;
}while(true);
```

In this example the ringbuffer is read until the end by using iteration.

```c
uintptr_t pos;
FUNCTION_RETURN_T ret;
float value;
ret = ringbuffer_get_first(buffer, &value, &pos);
while(ret == FUNCTION_RETURN_OK)
{
 // Do something with your value
 printf("%.2f\n", value);

 ret = ringbuffer_get_next(buffer, &value, &pos);
}
```
