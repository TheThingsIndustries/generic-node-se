# sensosrs_lorawan application

## Setup


The application can be used with the following Javascript payload formatter

```javascript
function decodeUplink(input) {
  var data = {};
  data.batt_volt = (input.bytes[0]/10);
  data.temperature = ((input.bytes[1] << 8) + input.bytes[2])/10;
  data.humidity = ((input.bytes[3] << 8) + input.bytes[4])/10;

  return {
    data: data,
  };
}
```
Please see [The Things Stack Javascript payload formatter documentation](https://www.thethingsindustries.com/docs/integrations/payload-formatters/javascript/) for more information.
