from encoding import trace

'''
lower    upper
0  1     7  4

5  2     6  3
'''

hand = trace.Trace()

hand.s_move((0, 0, 3))
hand.fission((1, 0, 0), 3)  # 0(1-7) -> 0(5-7) 1(2-4)

hand.fission((0, 0, 1), 1)  # 0(5-7) -> 0(7) 5(6)
hand.s_move((15, 0, 0))     # 1

hand.fission((0, 1, 0), 0)  # 0(7) -> 0() 7()
hand.s_move((3, 0, 0))      # 1
hand.s_move((0, 0, 12))     # 5

hand.wait()                 # 0
hand.fission((0, 0, 1), 1)  # 1(2-4) -> 1(4) 2(3)
hand.fission((0, 1, 0), 0)  # 5(6) -> 5() 6()
hand.s_move((0, 9, 0))      # 7

hand.wait()
hand.fission((0, 1, 0), 0)  # 1(4) -> 1() 4()
hand.s_move((0, 0, 12))     # 2
hand.wait()                 # 5
hand.s_move((0, 9, 0))      # 6
hand.wait()

hand.wait()
hand.wait()
hand.fission((0, 1, 0), 0)  # 2(3) -> 2() 3()
hand.s_move((0, 9, 0))      # 4
hand.wait()
hand.wait()
hand.wait()

hand.wait()
hand.wait()
hand.wait()
hand.s_move((0, 9, 0))      # 3
hand.wait()
hand.wait()
hand.wait()
hand.wait()

hand.gfill((1, 0, 1), (17, 10, 0))
hand.gfill((-1, 0, 1), (-17, 10, 0))
hand.gfill((-1, 0, -1), (-17, 10, 0))
hand.gfill((-1, 0, -1), (-17, -10, 0))
hand.gfill((-1, 0, 1), (-17, -10, 0))
hand.gfill((1, 0, -1), (17, 10, 0))
hand.gfill((1, 0, -1), (17, -10, 0))
hand.gfill((1, 0, 1), (17, -10, 0))

hand.gfill((1, 0, 1), (0, 10, 11))
hand.gfill((-1, 0, 1), (0, 10, 11))
hand.gfill((-1, 0, -1), (0, 10, -11))
hand.gfill((-1, 0, -1), (0, -10, -11))
hand.gfill((-1, 0, 1), (0, -10, 11))
hand.gfill((1, 0, -1), (0, 10, -11))
hand.gfill((1, 0, -1), (0, -10, -11))
hand.gfill((1, 0, 1), (0, -10, 11))


'''
lower    upper
0  1     7  4

5  2     6  3

3: (19, 16)
4: (19, 3)
6: (0, 16)
7: (0, 3)

'''

hand.wait()
hand.wait()
hand.s_move((0, 0, -12))
hand.gfill((-1, 0, -1), (-17, 0, -11))
hand.gfill((-1, 0, 1), (-17, 0, 11))
hand.s_move((0, 0, -12))
hand.gfill((1, 0, -1), (17, 0, -11))
hand.gfill((1, 0, 1), (17, 0, 11))

'''
0: (0, 3)
1: (19, 3)
2: (19, 4)
5: (0, 4)
'''

hand.fusion_p((0, 0, 1))
hand.fusion_p((0, 0, 1))
hand.fusion_s((0, 0, -1))
hand.l_move((0, 1, 0), (0, 0, -5))
hand.l_move((0, 1, 0), (0, 0, 3))
hand.fusion_s((0, 0, -1))
hand.l_move((0, 1, 0), (0, 0, -5))
hand.l_move((0, 1, 0), (0, 0, 3))

hand.wait()
hand.s_move((-15, 0, 0))
hand.s_move((-3, 0, 0))
hand.s_move((-3, 0, 0))
hand.s_move((9, 0, 0))
hand.s_move((9, 0, 0))

hand.wait()
hand.s_move((-3, 0, 0))
hand.gvoid((0, -1, -1), (-7, 0, -3))
hand.gvoid((0, -1, 1), (-7, 0, 3))
hand.gvoid((0, -1, -1), (7, 0, -3))
hand.gvoid((0, -1, 1), (7, 0, 3))

hand.fusion_p((1, 0, 0))
hand.fusion_s((-1, 0, 0))
hand.l_move((-5, 0, 0), (0, 0, -4))
hand.s_move((-5, 0, 0))
hand.l_move((1, 0, 0), (0, 0, -4))
hand.s_move((1, 0, 0))

hand.s_move((0, 0, -3))
hand.fusion_s((0, 0, -1))  # 3
hand.fusion_p((0, 0, 1))   # 4
hand.fusion_s((0, 0, -1))  # 5
hand.fusion_p((0, 0, 1))   # 6

hand.wait()
hand.fusion_s((-1, 0, 0))   # 4
hand.fusion_p((1, 0, 0))    # 7 (10,6)

hand.wait()
hand.s_move((-10, 0, 0))

hand.wait()
hand.s_move((0, 0, -6))

hand.wait()
hand.s_move((0, -10, 0))

hand.fusion_p((0, 1, 0))
hand.fusion_s((0, -1, 0))

hand.halt()

hand.encode('FA006.nbt')
