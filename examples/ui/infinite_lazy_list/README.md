# Infinite Lazy List

@auiexample{ui}
Usage of IListModel to make an infinite lazy list.

This example demonstrates making an optimal infinite list. That is, when you infinitely scroll down to load new items,
the older items are unloaded from memory.

The "loading" action is performed on a worker thread and a delay is simulated by AThread::sleep.

As a bonus, we've added a @ref ASpinnerV2 "spinner" to indicate that loading is in progress.

In this example, we've used functional style and decomposition technique instead of making custom view classes. The idea
is hide the implementation specifics of `myLazyList` that makes a basic AView in return. The overall state of produced
lazy list object is controlled by list model.

`myLazyList` is limited to specific type of model by intention. AUI does not provide some kind of generalization on its
own. In fact, we're demonstrating the whole concept in less than 100 lines of code. You certainly can make a
generalization that suits your project's needs.
