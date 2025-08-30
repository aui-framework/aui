# 7GUIs Temperature Converter

<!-- aui:example 7guis -->
Fahrenheit to Celsius and vice versa.

![](imgs/wrjikjkedkncbjw.webp)

Challenges: bidirectional data flow, user-provided text input.

The task is to build a frame containing two textfields TC and TF representing the temperature in Celsius and Fahrenheit,
respectively. Initially, both TC and TF are empty. When the user enters a numerical value into TC the corresponding
value in TF is automatically updated and vice versa. When the user enters a non-numerical string into TC the value in TF
is not updated and vice versa. The formula for converting a temperature C in Celsius into a temperature F in Fahrenheit
is C = (F - 32) * (5/9) and the dual direction is F = C * (9/5) + 32.

Temperature Converter increases the complexity of Counter by having bidirectional data flow between the Celsius and
Fahrenheit inputs and the need to check the user input for validity. A good solution will make the bidirectional
dependency very clear with minimal boilerplate code.

<!-- aui:include examples/7guis/temperature_converter/src/main.cpp -->

## Comparison to Jetpack Compose

Here's implementation of the same app with Jetpack Compose:

```java
package ...

import ...

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            CounterTheme {
                Counter()
            }
        }
    }
}

@Composable
fun TemperatureBiConverter() {
    var celsiusInputData by remember { mutableStateOf("0.0") }
    var fahrenheitInputData by remember { mutableStateOf("32.0") }
    Row(
        modifier = Modifier
            .fillMaxSize()
            .padding(horizontal = 8.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.SpaceBetween
    ) {
        OutlinedTextField(
            modifier = Modifier.weight(1f),
            value = celsiusInputData,
            onValueChange = { input ->
                celsiusInputData = input
                input.toFloatOrNull()?.also {
                    fahrenheitInputData = (it * (9f / 5f) + 32f).toString()
                }
            },
            label = { Text("Celsius") }
        )
        Text(
            text = "=", fontSize = 18.sp,
            modifier = Modifier.padding(horizontal = 8.dp)
        )
        OutlinedTextField(
            modifier = Modifier.weight(1f),
            value = fahrenheitInputData,
            onValueChange = { input ->
                fahrenheitInputData = input
                input.toFloatOrNull()
                    ?.also {
                        celsiusInputData = ((it - 32) * (5f / 9f)).toString()
                    }
            },
            label = { Text("Fahrenheit") }
        )
    }
}
```
