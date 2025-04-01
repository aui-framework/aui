# 7GUIs Counter

@auiexample{7guis}
Simple counter.

@image html docs/imgs/Screenshot_20250401_081123.png

Challenge: Understanding the basic ideas of a language/toolkit.

The task is to build a frame containing a label or read-only textfield T and a button B. Initially, the value in T is
“0” and each click of B increases the value in T by one.

Counter serves as a gentle introduction to the basics of the language, paradigm and toolkit for one of the simplest GUI
applications imaginable. Thus, Counter reveals the required scaffolding and how the very basic features work together to
build a GUI application. A good solution will have almost no scaffolding.

@include examples/7guis/counter/src/main.cpp

# Comparison to Jetpack Compose

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
fun Counter() {
    var counter by remember { mutableIntStateOf(0) }
    Row(
        verticalAlignment = Alignment.CenterVertically,
        modifier = Modifier.fillMaxSize(),
        horizontalArrangement = Arrangement.SpaceAround
    ) {
        Text(text = "Counter $counter"),
        Button(onClick = { counter++ }) {
            Text(text = "Click to increment")
        }
    }
}
```

@image html docs/imgs/Screenshot_20250401_081455.png

|           | AUI                                                                           | Kotlin                            |
|-----------|-------------------------------------------------------------------------------|-----------------------------------|
| State     | Inside class                                                                  | Local variable (`remember`)       |
|           | `Vertical`                                                                    | `Column`                          |
|           | `Horizontal`                                                                  | `Row`                             |
| Display   | `AObject::connect(mCounter.readProjected(AString::number<int>), it->text());` | `Text(text = "Counter $counter")` |
| Increment | `Button { "Count" }.connect(&AView::clicked, [&] { mCounter += 1; })`         | `Button(onClick = { counter++ })` |


AUI tend to have longer expressions but does not differ that much. Jetpack Compose is an immediate mode UI, whereas AUI
is a retained mode UI.
