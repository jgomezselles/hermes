# Understanding hermes output

Once you execute hermes, you will find this in your console:
```bash
hermes-66547c85b6-55vl9:/hermes ./hermes -r1 -p1 -t12
Rate is 1req/s
Sending a request every 1e+06us
Time (s)      Sent/s    Recv/s        RT (ms)     minRT (ms)     maxRT (ms)           Sent        Success         Errors       Timeouts
Connected to test-server:8080
1.0              1.0       1.0          4.264          3.148          5.776              2              2              0              0
2.0              1.0       1.0          4.264          3.148          5.776              2              2              0              0
3.0              1.0       1.0          4.287          3.148          5.776              3              3              0              0
4.0              1.0       1.0          3.490          1.882          5.776              4              4              0              0
5.0              1.0       1.0          3.468          1.882          5.776              5              5              0              0
6.0              1.0       1.0          3.181          1.882          5.776              6              6              0              0
7.0              1.0       1.0          3.244          1.882          5.776              7              7              0              0
8.0              1.0       1.0          3.077          1.882          5.776              8              8              0              0
9.0              1.0       1.0          3.105          1.882          5.776              9              9              0              0
10.0             1.0       1.0          3.068          1.882          5.776             10             10              0              0
11.0             1.0       1.0          3.104          1.882          5.776             11             11              0              0
12.0             1.0       1.0          3.263          1.882          5.776             12             12              0              0
Execution finished. Printing stats...
Time (s)      Sent/s    Recv/s        RT (ms)     minRT (ms)     maxRT (ms)           Sent        Success         Errors       Timeouts
>>>message1<<<
12.0             0.5       0.5          3.915          3.337          5.776              6              6              0              0
>>>message2<<<
12.0             0.1       0.1          3.148          3.148          3.148              1              1              0              0
>>>message3<<<
12.0             0.2       0.2          2.000          1.882          2.125              2              2              0              0
>>>message4<<<
12.0             0.2       0.2          2.386          2.065          2.757              2              2              0              0
>>>message5<<<
12.0             0.1       0.1          5.653          5.653          5.653              1              1              0              0
>>>Total<<<
12.0             1.0       1.0          3.263          1.882          5.776             12             12              0              0
```

Keep in mind that all printed statistics are cumulative (not partials, so they take into
account all the values of your test) and printed every `p` seconds that you set in the
execution.

Output files are saved by default under “hermes.out.*”, containing:

* `hermes.out.accum` – Cumulative statistics (as the ones you saw in screen)
* `hermes.out.<message-id>` - Cumulative statistics for every message id
* `hermes.out.err` – Cumulative number and type of errors found at print-period “p”
* `hermes.out.partial` – Partial statistics for every print-period “p”.
This means the cumulative statistics between print-periods [pn, pn+1] for all pn
 

> Note: Custom error codes are reported by hermes when having reconnection issues and they are all numbered as 46X. They are not sent by the server, but noted as that when a request could not be sent due to a connection problem (your server most likely went down).
