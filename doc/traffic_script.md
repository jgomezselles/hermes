# Traffic script

Hermes sends traffic based on the definition of a json file. By default, it is loaded from
`/etc/scripts/traffic.json`, and needs to be compliant with its schema definition. The
schema can be accessed by executing: `./hermes -s`. It is also thrown to the console if the
validation fails.

In brief, what you need is (maybe better to look at example or the schema, **an API definition is coming**):

* `dns`: `string` - your server address
* `port`: `string` - your server port
* `timeout`: `integer` - the number of ms to wait until non answered requests are considered to be a timeout error
* `flow`: `array of strings` – the name or id of the messages, in order, that define your traffic. For example: `[“request1”, “request2”]`.
* `messages`: `json object` – the definition of each one of the messages defined in your `flow`. Every element mentioned under flow, must be defined as an object inside this field, named after its id, with the following content:
    * `method`: `string` – Http method for this message (`“POST”`, `“GET”`…)
    * `url`: `string` – The url for your request. Do not start with “/”!
    * `body`: `json object` – The body of your request
    * `response`: `json object` – must contain:
        * `code`: `integer` – the http response code used to consider that the request was successful, once answered
    * `save_from_answer` – `json object`, **Optional**: used to pass information from an answer to a subsequent request containing:
        * `name`: `string` – an id you want to give to the chunk of the response you want to save
        * `path`: `string` – the path where lies the chunk of the answer you want to save from the response
        * `value_type`: `string` – The type of the chunk of answer you want to save (only `string`, `int` or `object` supported right now)
    * `add_from_saved_to_body` – `json object`, **Optional**: used to construct a request based on a previously stored json fragment from a `save_from_answer` object:
        * `name`: `string` – the id you gave to a previous `save_from_answer`
        * `path`: `string` – the path where you want to add the json fragment into the new request
        * `value_type`: `string` – The type of the fragment (only `string`, `int` or `object` supported right now)
* `ranges`: `json object` – **Optional:** Use this feature if you want the scripts to iterate over certain values. Each property you add here with a name (e.g. *"my_prop”*) will be expanded when such name is found under angle braces (e.g. blabla<my_prop>blabla) in both `url` and `body` of each message. Each of those defined names must contain:
    * `min`: `integer` – minimum value of the iteration
    * `max`: `integer` – maximum value of the iteration
 
For a given script, “request2” will be never sent before “request1” has been answered.
If a new request is needed to be sent before that happens, a new script is initialized.
When ranges are defined, a new value of the range is taken for every initialized script.