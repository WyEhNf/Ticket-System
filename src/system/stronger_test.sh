[1] add_user -u admin -p admin -n 管理员 -m admin@mail.com -g 10
[2] login -u admin -p admin
[3] add_user -c admin -u alice -p 123 -n 爱丽丝 -m alice@mail.com -g 1
[4] add_user -c admin -u bob -p 456 -n 鲍勃 -m bob@mail.com -g 1
[5] login -u alice -p 123
[6] login -u bob -p 456
[7] add_train -i T100 -n 3 -m 100 -s A站|B站|C站 -p 100|200 -x 08:00 -t 300|300 -o 10 -d 06-01|06-30 -y G
[8] add_train -i T200 -n 3 -m 50 -s A站|D站|C站 -p 150|150 -x 09:00 -t 200|200 -o 5 -d 06-01|06-30 -y D
[9] release_train -i T100
[10] release_train -i T200
[11] query_train -i T100 -d 06-10
[12] query_ticket -s A站 -t C站 -d 06-10 -p cost
[13] buy_ticket -u alice -i T100 -d 06-10 -n 80 -f A站 -t C站
[14] buy_ticket -u bob -i T100 -d 06-10 -n 30 -f A站 -t C站 -q true
[15] query_order -u alice
[16] query_order -u bob
[17] refund_ticket -u alice
[18] query_order -u bob
[19] query_ticket -s A站 -t C站 -d 06-10 -p time
[20] logout -u alice
[21] logout -u bob
[22] logout -u admin
[23] clean
[24] exit