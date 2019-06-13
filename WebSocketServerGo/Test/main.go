package main

type structabc struct {
	name string
	val  string
}

func structabcf(a structabc) {
	a.name = "abc"
	a.val = "def"
}

func structabcf1(a *structabc) {
	a.name = "abc1"
	a.val = "def1"
}

func ch1(a [4]int) {
	a[0] = 5
}

func main() {
	//a := structabc{"a", "b"}
	//structabcf(a)
	//structabcf1(&a)
	var a = [4]int{1, 2, 3, 4}
	//println(a)
	ch1(a)
	//println(a)

}
