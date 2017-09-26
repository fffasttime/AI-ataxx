from loadataxxdata import ataxxdata
datas = ataxxdata("ataxxdata2_4ne_.txt")
valids = ataxxdata("2_va.txt")
import tensorflow as tf
sess = tf.InteractiveSession()

def weight_variable(shape):
    initial = tf.truncated_normal(shape, stddev=0.1)
    return tf.Variable(initial)

def bias_variable(shape):
    initial = tf.constant(0.1, shape=shape)
    return tf.Variable(initial)

def conv2d(x, W):
    return tf.nn.conv2d(x, W, strides=[1, 1, 1, 1], padding='VALID')

#def max_pool_2x2(x):
#    return tf.nn.max_pool(x, ksize=[1, 2, 2, 1],strides=[1, 2, 2, 1], padding='VALID')  
                        
x = tf.placeholder(tf.float32, [None, 49, 3])
x_col = tf.placeholder(tf.float32,[None, 1])
y_ = tf.placeholder(tf.float32, [None, 1])
x_image = tf.reshape(x, [-1,7,7,3])
                        
W_conv1 = weight_variable([3, 3, 3, 32])
b_conv1 = bias_variable([32])

h_conv1 = tf.nn.relu(conv2d(x_image, W_conv1) + b_conv1)

W_conv2 = weight_variable([3, 3, 32, 64])
b_conv2 = bias_variable([64])
h_conv2 = tf.nn.relu(conv2d(h_conv1, W_conv2) + b_conv2)
W_fc1 = weight_variable([3 * 3 * 64, 256])
b_fc1 = bias_variable([256])
h_conv2_flat = tf.reshape(h_conv2, [-1, 3*3*64])
#x_col_t=tf.concat(1,[x_col,x_col,x_col,x_col])
#h_conv2_flat=tf.concat(1,[h_conv2_flat,x_col_t])

h_fc1 = tf.nn.relu(tf.matmul(h_conv2_flat, W_fc1) + b_fc1)

#keep_prob = tf.placeholder(tf.float32)
#h_fc1_drop = tf.nn.dropout(h_fc1, keep_prob)

W_fc2 = weight_variable([256, 64])
b_fc2 = bias_variable([64])
h_fc2=tf.nn.relu(tf.matmul(h_fc1, W_fc2) + b_fc2)

W_fc3 = weight_variable([64, 1])
b_fc3 = bias_variable([1])
y=tf.matmul(h_fc2, W_fc3) + b_fc3

loss = tf.reduce_mean(tf.square(y-y_))
l1_eps=tf.reduce_mean(tf.abs(y-y_))
train_step = tf.train.AdamOptimizer(0.005).minimize(loss)
acc=tf.reduce_mean(tf.cast(tf.equal(tf.sign(y-1),tf.sign(y_)),tf.float32))
mean=tf.reduce_mean(y)
tf.global_variables_initializer().run()
for i in range(10000):
    batch = datas.next_batch(100)
    if i%100 == 0:

        valid_loss = l1_eps.eval(feed_dict={x:valids.data[1], x_col:valids.data[0], y_: valids.data[2]})
        print("step %d, valid loss %g"%(i, valid_loss))
        
        train_loss = l1_eps.eval(feed_dict={x:batch[1], x_col:batch[0], y_: batch[2]})
        print("step %d, train loss %g"%(i, train_loss))
    
    train_step.run(feed_dict={x:batch[1], x_col:batch[0], y_: batch[2]})
