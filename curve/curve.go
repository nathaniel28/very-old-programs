package main

import (
	"fmt"
	"math"
)

type Point struct {
	X, Y float64
}

type Path struct {
	Points  []Point
	Subpath *Path
}

func Dist(a, b *Point) float64 {
	return math.Sqrt(math.Pow(a.X-b.X, 2) + math.Pow(a.Y-b.Y, 2))
}

func Slope(a, b *Point) float64 {
	return (a.Y - b.Y) / (a.X - b.X)
}

func (pnt *Point) SetBetween(a, b *Point, percent float64) {
	slope := Slope(a, b)
	var m float64
	if a.X <= b.X {
		m = 1.0
	} else {
		m = -1.0
	}
	pnt.X = a.X + (m*(percent*Dist(a, b)))/math.Sqrt(1+math.Pow(slope, 2))
	pnt.Y = slope*(pnt.X-a.X) + a.Y
}

func (path *Path) PathInit(points []Point) {
	path.Points = points
	if len(points) > 1 {
		var subpath Path
		subpathPoints := make([]Point, len(points)-1)
		copy(subpathPoints, points)
		subpath.PathInit(subpathPoints)
		path.Subpath = &subpath
	}
}

func (path *Path) Update(percent float64) Point {
	if path.Subpath == nil {
		return path.Points[0]
	}

	for i := 0; i < len(path.Subpath.Points); i++ {
		path.Subpath.Points[i].SetBetween(&path.Points[i], &path.Points[i+1], percent)
	}

	return path.Subpath.Update(percent)
}

func (path *Path) CalculateCurve(step float64) []Point {
	var curvePoints []Point
	for i := float64(0); i < 1; i += step {
		curvePoints = append(curvePoints, path.Update(i))
	}
	return curvePoints
}

func main() {
	var curve Path
	curve.PathInit([]Point{Point{2, 5}, Point{4, 3}, Point{5, 4}, Point{7, 1}})

	points := curve.CalculateCurve(0.01)

	for _, point := range points {
		fmt.Println(point)
	}
}
