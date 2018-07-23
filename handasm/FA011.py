from encoding import trace

'''
lower    upper
0  1     7  4

5  2     6  3
'''

hand = trace.Trace()

hand.s_move((6, 0, 0))
hand.s_move((0, 0, 6))
hand.fission((1, 0, 0), 3)  # 0(1-7) -> 0(5-7) 1(2-4)

hand.fission((0, 0, 1), 1)  # 0(5-7) -> 0(7) 5(6)
hand.s_move((5, 0, 0))      # 1

hand.fission((0, 1, 0), 0)  # 0(7) -> 0() 7()
hand.fission((0, 0, 1), 1)  # 1(2-4) -> 1(4) 2(3)
hand.s_move((0, 0, 5))      # 5

hand.wait()                 # 0
hand.fission((0, 1, 0), 0)  # 1(4) -> 1() 4()
hand.s_move((0, 0, 5))      # 2
hand.fission((0, 1, 0), 0)  # 5(6) -> 5() 6()
hand.s_move((0, 15, 0))     # 7

hand.wait()
hand.wait()
hand.fission((0, 1, 0), 0)  # 2(3) -> 2() 3()
hand.s_move((0, 15, 0))     # 4
hand.wait()                 # 5
hand.s_move((0, 15, 0))     # 6
hand.s_move((0, 2, 0))      # 7

hand.wait()
hand.wait()
hand.wait()
hand.s_move((0, 15, 0))     # 3
hand.s_move((0, 2, 0))      # 4
hand.wait()
hand.s_move((0, 2, 0))      # 6
hand.wait()                 # 7

hand.wait()
hand.wait()
hand.wait()
hand.s_move((0, 2, 0))      # 3
hand.wait()
hand.wait()
hand.wait()
hand.wait()

hand.gfill((1, 0, 1), (4, 18, 0))
hand.gfill((-1, 0, 1), (-4, 18, 0))
hand.gfill((-1, 0, -1), (-4, 18, 0))
hand.gfill((-1, 0, -1), (-4, -18, 0))
hand.gfill((-1, 0, 1), (-4, -18, 0))
hand.gfill((1, 0, -1), (4, 18, 0))
hand.gfill((1, 0, -1), (4, -18, 0))
hand.gfill((1, 0, 1), (4, -18, 0))

hand.gfill((1, 0, 1), (0, 18, 4))
hand.gfill((-1, 0, 1), (0, 18, 4))
hand.gfill((-1, 0, -1), (0, 18, -4))
hand.gfill((-1, 0, -1), (0, -18, -4))
hand.gfill((-1, 0, 1), (0, -18, 4))
hand.gfill((1, 0, -1), (0, 18, -4))
hand.gfill((1, 0, -1), (0, -18, -4))
hand.gfill((1, 0, 1), (0, -18, 4))

hand.gfill((1, 0, 1), (4, 0, 4))
hand.gfill((-1, 0, 1), (-4, 0, 4))
hand.gfill((-1, 0, -1), (-4, 0, -4))
hand.gfill((-1, 0, -1), (-4, 0, -4))
hand.gfill((-1, 0, 1), (-4, 0, 4))
hand.gfill((1, 0, -1), (4, 0, -4))
hand.gfill((1, 0, -1), (4, 0, -4))
hand.gfill((1, 0, 1), (4, 0, 4))

hand.wait()
hand.wait()
hand.wait()
hand.s_move((0, -15, 0))
hand.s_move((0, -15, 0))
hand.wait()
hand.s_move((0, -15, 0))
hand.s_move((0, -15, 0))

hand.wait()
hand.wait()
hand.wait()
hand.s_move((0, -2, 0))
hand.s_move((0, -2, 0))
hand.wait()
hand.s_move((0, -2, 0))
hand.s_move((0, -2, 0))

hand.fusion_p((0, 1, 0))
hand.fusion_p((0, 1, 0))
hand.fusion_p((0, 1, 0))
hand.fusion_s((0, -1, 0))
hand.fusion_s((0, -1, 0))
hand.fusion_p((0, 1, 0))
hand.fusion_s((0, -1, 0))
hand.fusion_s((0, -1, 0))

hand.wait()
hand.wait()
hand.s_move((0, 0, -5))
hand.s_move((0, 0, -5))

hand.fusion_p((0, 0, 1))
hand.fusion_p((0, 0, 1))
hand.fusion_s((0, 0, -1))
hand.fusion_s((0, 0, -1))

hand.wait()
hand.s_move((-5, 0, 0))

hand.fusion_p((1, 0, 0))
hand.fusion_s((-1, 0, 0))

hand.s_move((-6, 0, 0))
hand.s_move((0, 0, -6))
hand.halt()

hand.encode('FA011.nbt')
